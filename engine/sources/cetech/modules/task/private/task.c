//==============================================================================
// Includes
//==============================================================================

#include <cetech/core/os/thread.h>
#include <cetech/core/memory.h>

#include <cetech/modules/develop_system/develop.h>
#include <cetech/core/config.h>
#include <cetech/modules/resource/resource.h>
#include <cetech/core/module.h>

#include <cetech/modules/task/task.h>
#include <cetech/core/api.h>
#include <cetech/core/os/cpu.h>

#include "task_queue.h"

//==============================================================================
// Defines
//==============================================================================

#define make_task(i) (task_t){.id = i}

#define MAX_TASK 4096
#define LOG_WHERE "taskmanager"

//==============================================================================
// Globals
//==============================================================================

struct task {
    void *data;
    task_work_t task_work;
    const char *name;
    enum task_affinity affinity;
};

static __thread char _worker_id = 0;

static struct G {
    struct task _task_pool[MAX_TASK];
    struct task_queue _workers_queue[TASK_MAX_WORKERS];
    thread_t _workers[TASK_MAX_WORKERS - 1];
    struct task_queue _gloalQueue;
    atomic_int _task_pool_idx;
    int _workers_count;
    int _Run;
} TaskManagerGlobal = {0};

#define _G TaskManagerGlobal


IMPORT_API(develop_api_v0);
IMPORT_API(memory_api_v0);
IMPORT_API(thread_api_v0);
IMPORT_API(cpu_api_v0);
IMPORT_API(log_api_v0);

//==============================================================================
// Private
//==============================================================================

typedef struct {
    uint32_t id;
} task_t;

static const task_t task_null = (task_t) {.id = 0};

static task_t _new_task() {
    int idx = atomic_fetch_add(&_G._task_pool_idx, 1);

    if ((idx & (MAX_TASK - 1)) == 0) {
        idx = atomic_fetch_add(&_G._task_pool_idx, 1);
    }

    return make_task(idx & (MAX_TASK - 1));
}

static void _push_task(task_t t) {
    CETECH_ASSERT("", t.id != 0);

    int affinity = _G._task_pool[t.id].affinity;

    struct task_queue *q;
    switch (_G._task_pool[t.id].affinity) {
        case TASK_AFFINITY_NONE:
            q = &_G._gloalQueue;
            break;

        default:
            q = &_G._workers_queue[affinity - 2];
            break;
    }

    queue_task_push(q, t.id);
}


static task_t _try_pop(struct task_queue *q) {
    uint32_t poped_task;

    if (!queue_task_size(q)) {
        return task_null;
    }

    if (queue_task_pop(q, &poped_task, 0)) {
        if (poped_task != 0) {
            return make_task(poped_task);
        }
    }

    return task_null;
}

static void _mark_task_job_done(task_t task) {

}

static task_t _task_pop_new_work() {
    task_t popedTask;

    struct task_queue *qw = &_G._workers_queue[_worker_id];
    struct task_queue *qg = &_G._gloalQueue;

    popedTask = _try_pop(qw);
    if (popedTask.id != 0) {
        return popedTask;
    }

    popedTask = _try_pop(qg);
    if (popedTask.id != 0) {
        return popedTask;
    }

    return task_null;
}

int taskmanager_do_work();

static int _task_worker(void *o) {
    // Wait for run signal 0 -> 1
    while (!_G._Run) {
    }

    _worker_id = (char) o;

    log_api_v0.log_debug("task_worker", "Worker %d init", _worker_id);

    while (_G._Run) {
        if (!taskmanager_do_work()) {
            thread_api_v0.yield();
        }
    }

    log_api_v0.log_debug("task_worker", "Worker %d shutdown", _worker_id);
    return 1;
}




//==============================================================================
// Interface
//==============================================================================

void taskmanager_add(struct task_item *items,
                     uint32_t count) {
    for (uint32_t i = 0; i < count; ++i) {
        task_t task = _new_task();
        _G._task_pool[task.id] = (struct task) {
                .name = items[i].name,
                .task_work = items[i].work,
                .affinity = items[i].affinity,
        };

        _G._task_pool[task.id].data = items[i].data;

        _push_task(task);
    }
}

int taskmanager_do_work() {
    task_t t = _task_pop_new_work();

    if (t.id == 0) {
        return 0;
    }

    struct scope_data sd = develop_api_v0.enter_scope(
            _G._task_pool[t.id].name);

    _G._task_pool[t.id].task_work(_G._task_pool[t.id].data);

    develop_api_v0.leave_scope(sd);

    _mark_task_job_done(t);

    return 1;
}

void taskmanager_wait_atomic(atomic_int *signal,
                             uint32_t value) {
    while (atomic_load_explicit(signal, memory_order_acquire) == value) {
        taskmanager_do_work();
    }
}

char taskmanager_worker_id() {
    return _worker_id;
}

int taskmanager_worker_count() {
    return _G._workers_count;
}

static void _init_api(struct api_v0* api){
    static struct task_api_v0 _api = {
            .worker_count = taskmanager_worker_count,
            .add = taskmanager_add,
            .do_work = taskmanager_do_work,
            .wait_atomic = taskmanager_wait_atomic,
            .worker_id = taskmanager_worker_id
    };

    api->register_api("task_api_v0", &_api);
}

static void _init( struct api_v0* api) {
    GET_API(api, develop_api_v0);
    GET_API(api, memory_api_v0);
    GET_API(api, thread_api_v0);
    GET_API(api, log_api_v0);
    GET_API(api, cpu_api_v0);



    _G = (struct G) {0};

    int core_count = cpu_api_v0.count();

    static const uint32_t main_threads_count = 1;
    const uint32_t worker_count = core_count - main_threads_count;

    log_api_v0.log_info("task", "Core/Main/Worker: %d, %d, %d", core_count,
             main_threads_count, worker_count);

    _G._workers_count = worker_count;

    queue_task_init(&_G._gloalQueue, MAX_TASK, memory_api_v0.main_allocator());

    for (int i = 0; i < worker_count + 1; ++i) {
        queue_task_init(&_G._workers_queue[i], MAX_TASK,
                        memory_api_v0.main_allocator());
    }

    for (int j = 0; j < worker_count; ++j) {
        _G._workers[j] = thread_api_v0.create((thread_fce_t) _task_worker,
                                              "worker",
                                              (void *) ((intptr_t) (j + 1)));
    }

    _G._Run = 1;
}

static void _shutdown() {
    _G._Run = 0;

    int status = 0;

    for (uint32_t i = 0; i < _G._workers_count; ++i) {
        //thread_kill(_G._workers[i]);
        thread_api_v0.wait(_G._workers[i], &status);
    }

    queue_task_destroy(&_G._gloalQueue);

    for (int i = 0; i < _G._workers_count + 1; ++i) {
        queue_task_destroy(&_G._workers_queue[i]);
    }

    _G = (struct G) {0};
}

void *task_get_module_api(int api) {


    switch (api) {
        case PLUGIN_EXPORT_API_ID: {
            static struct module_api_v0 module = {0};

            module.init = _init;
            module.init_api = _init_api;
            module.shutdown = _shutdown;

            return &module;
        }

        default:
            return NULL;
    }

}