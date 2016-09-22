//==============================================================================
// Includes
//==============================================================================

#include <engine/components/types.h>
#include <engine/core/world_system.h>
#include <celib/os/vio.h>
#include <celib/stringid/types.h>
#include <engine/core/resource_manager.h>
#include <celib/stringid/stringid.h>
#include <engine/develop/resource_compiler.h>
#include <engine/core/unit_system.h>
#include <engine/core/entcom.h>
#include <engine/components/transform.h>
#include <celib/math/quatf.h>
#include "engine/core/memory_system.h"
#include "celib/containers/map.h"

//==============================================================================
// Typedefs
//==============================================================================

ARRAY_T(world_t);
ARRAY_PROTOTYPE(entity_t);
ARRAY_PROTOTYPE(world_callbacks_t);
ARRAY_PROTOTYPE(stringid64_t);

MAP_PROTOTYPE(entity_t);
MAP_T(world_t);

//==============================================================================
// GLobals
//==============================================================================

struct level_instance {
    entity_t level_entity;
    MAP_T(entity_t) spawned_entity;
};

#define _G WorldGlobals
static struct G {
    ARRAY_T(world_callbacks_t) callbacks;
    struct handlerid world_handler;
    stringid64_t level_type;
} _G = {0};

struct level_instance *_new_level_instance() {
    return NULL;
}


//==============================================================================
// Resource
//==============================================================================

void *level_resource_loader(struct vio *input,
                            struct allocator *allocator) {
    const i64 size = vio_size(input);
    char *data = CE_ALLOCATE(allocator, char, size);
    vio_read(input, data, 1, size);

    return data;
}

void level_resource_unloader(void *new_data,
                             struct allocator *allocator) {
    CE_DEALLOCATE(allocator, new_data);
}

void level_resource_online(stringid64_t name,
                           void *data) {
}

void level_resource_offline(stringid64_t name,
                            void *data) {
}

void *level_resource_reloader(stringid64_t name,
                              void *old_data,
                              void *new_data,
                              struct allocator *allocator) {
    level_resource_offline(name, old_data);
    level_resource_online(name, new_data);

    CE_DEALLOCATE(allocator, old_data);

    return new_data;
}

static const resource_callbacks_t _level_resource_defs = {
        .loader = level_resource_loader,
        .unloader = level_resource_unloader,
        .online = level_resource_online,
        .offline = level_resource_offline,
        .reloader = level_resource_reloader
};

struct foreach_units_data {
    const char *filename;
    struct compilator_api *capi;
    ARRAY_T(stringid64_t) *names;
    ARRAY_T(u32) *offset;
    ARRAY_T(u8) *data;
};

void forach_units_clb(yaml_node_t key,
                      yaml_node_t value,
                      void *_data) {
    struct foreach_units_data *data = _data;

    char name[128] = {0};
    yaml_as_string(key, name, CE_ARRAY_LEN(name));

    ARRAY_PUSH_BACK(stringid64_t, data->names, stringid64_from_string(name));
    ARRAY_PUSH_BACK(u32, data->offset, ARRAY_SIZE(data->data));

    unit_resource_compiler(value, data->filename, data->data, data->capi);
}

struct level_blob {
    u32 units_count;
    // res  + 1             : stringid64_t names[units_count]
    // names  + units_count : u32 offset[units_count]
    // offset + units_count : u8 data[*]
};


#define level_blob_names(r) ((stringid64_t*) ((r) + 1))
#define level_blob_offset(r) ((u32*) ((level_blob_names(r)) + (r)->units_count))
#define level_blob_data(r)   ((u8*) ((level_blob_offset(r)) + (r)->units_count))

int _level_resource_compiler(const char *filename,
                             struct vio *source_vio,
                             struct vio *build_vio,
                             struct compilator_api *compilator_api) {

    char source_data[vio_size(source_vio) + 1];
    memory_set(source_data, 0, vio_size(source_vio) + 1);
    vio_read(source_vio, source_data, sizeof(char), vio_size(source_vio));

    yaml_document_t h;
    yaml_node_t root = yaml_load_str(source_data, &h);

    yaml_node_t units = yaml_get_node(root, "units");

    ARRAY_T(stringid64_t) names;
    ARRAY_T(u32) offset;
    ARRAY_T(u8) data;

    ARRAY_INIT(stringid64_t, &names, memsys_main_allocator());
    ARRAY_INIT(u32, &offset, memsys_main_allocator());
    ARRAY_INIT(u8, &data, memsys_main_allocator());

    struct foreach_units_data unit_data = {
            .names = &names,
            .offset = &offset,
            .data = &data,
            .capi = compilator_api,
            .filename = filename
    };

    yaml_node_foreach_dict(units, forach_units_clb, &unit_data);

    struct level_blob res = {
            .units_count = ARRAY_SIZE(&names)
    };

    vio_write(build_vio, &res, sizeof(struct level_blob), 1);
    vio_write(build_vio, &ARRAY_AT(&names, 0), sizeof(stringid64_t), ARRAY_SIZE(&names));
    vio_write(build_vio, &ARRAY_AT(&offset, 0), sizeof(u32), ARRAY_SIZE(&offset));
    vio_write(build_vio, &ARRAY_AT(&data, 0), sizeof(u8), ARRAY_SIZE(&data));

    ARRAY_DESTROY(stringid64_t, &names);
    ARRAY_DESTROY(u32, &offset);
    ARRAY_DESTROY(u8, &data);

    return 1;
}


//==============================================================================
// Public interface
//==============================================================================

int world_init(int stage) {
    if (stage == 0) {
        return 1;
    }

    _G = (struct G) {0};

    ARRAY_INIT(world_callbacks_t, &_G.callbacks, memsys_main_allocator());

    handlerid_init(&_G.world_handler, memsys_main_allocator());

    _G.level_type = stringid64_from_string("level");

    resource_register_type(_G.level_type, _level_resource_defs);
    resource_compiler_register(_G.level_type, _level_resource_compiler);

    return 1;
}

void world_shutdown() {
    handlerid_destroy(&_G.world_handler);
    ARRAY_DESTROY(world_callbacks_t, &_G.callbacks);
    _G = (struct G) {0};
}

void world_register_callback(world_callbacks_t clb) {
    ARRAY_PUSH_BACK(world_callbacks_t, &_G.callbacks, clb);
}

world_t world_create() {
    world_t w = {.h = handlerid_handler_create(&_G.world_handler)};

    for (int i = 0; i < ARRAY_SIZE(&_G.callbacks); ++i) {
        ARRAY_AT(&_G.callbacks, i).on_created(w);
    }

    return w;
}

void world_destroy(world_t world) {
    for (int i = 0; i < ARRAY_SIZE(&_G.callbacks); ++i) {
        ARRAY_AT(&_G.callbacks, i).on_destroy(world);
    }

    handlerid_handler_destroy(&_G.world_handler, world.h);
}


void world_load_level(world_t world,
                      stringid64_t name) {
    struct level_blob *res = resource_get(_G.level_type, name);

    stringid64_t *names = level_blob_names(res);
    u32 *offset = level_blob_offset(res);
    u8 *data = level_blob_data(res);

    entity_t level_ent = entity_manager_create();

    transform_t t = transform_create(world, level_ent, (entity_t) {UINT32_MAX}, (vec3f_t) {0}, QUATF_IDENTITY,
                                     (vec3f_t) {{1.0f, 1.0f, 1.0f}});

    for (int i = 0; i < res->units_count; ++i) {
        entity_t unit = unit_spawn_from_resource(world, (void *) &data[offset[i]]);

        if (transform_has(world, unit)) {
            transform_link(world, level_ent, unit);
        }
    }
}
