//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>

#include <cetech/core/container/map.inl>
#include <cetech/core/module.h>
#include <cetech/core/memory.h>
#include <cetech/core/yaml.h>
#include <cetech/core/hash.h>
#include <cetech/core/handler.h>
#include <cetech/core/os/path.h>
#include <cetech/core/os/vio.h>

#include <cetech/core/config.h>
#include <cetech/modules/resource/resource.h>
#include <cetech/modules/world/world.h>
#include <cetech/modules/entity/entity.h>
#include <cetech/modules/component/component.h>
#include <cetech/core/api.h>


//==============================================================================
// Globals
//==============================================================================

ARRAY_PROTOTYPE(entity_t);
MAP_PROTOTYPE(entity_t);
ARRAY_PROTOTYPE_N(struct array_entity_t, array_entity_t);
MAP_PROTOTYPE_N(struct array_entity_t, array_entity_t);

#define _G EntityMaagerGlobals
static struct G {
    struct handler32gen *entity_handler;
    MAP_T(uint32_t) spawned_map;
    ARRAY_T(array_entity_t) spawned_array;
    uint64_t type;
} _G = {0};

IMPORT_API(memory_api_v0);
IMPORT_API(component_api_v0);
IMPORT_API(resource_api_v0);
IMPORT_API(handler_api_v0);
IMPORT_API(path_v0);
IMPORT_API(log_api_v0);
IMPORT_API(vio_api_v0);
IMPORT_API(hash_api_v0);


struct entity_resource {
    uint32_t ent_count;
    uint32_t comp_type_count;
    //uint64_t parents [ent_count]
    //uint64_t comp_types [comp_type_count]
    //component_data cdata[comp_type_count]
};

struct component_data {
    uint32_t ent_count;
    uint32_t size;
    // uint32_t ent[ent_count];
    // char data[ent_count];
};

#define entity_resource_parents(r) ((uint32_t*)((r) + 1))
#define entity_resource_comp_types(r) ((uint64_t*)(entity_resource_parents(r) + ((r)->ent_count)))
#define entity_resource_comp_data(r) ((struct component_data*)(entity_resource_comp_types(r) + ((r)->comp_type_count)))

#define component_data_ent(cd) ((uint32_t*)((cd) + 1))
#define component_data_data(cd) ((char*)((component_data_ent(cd) + ((cd)->ent_count))))

uint32_t _new_spawned_array() {
    uint32_t idx = ARRAY_SIZE(&_G.spawned_array);

    ARRAY_PUSH_BACK(array_entity_t, &_G.spawned_array,
                    (struct array_entity_t) {0});
    ARRAY_T(entity_t) *array = &ARRAY_AT(&_G.spawned_array, idx);

    ARRAY_INIT(entity_t, array, memory_api_v0.main_allocator());
    return idx;
}

void _map_spawned_array(entity_t root,
                        uint32_t idx) {
    MAP_SET(uint32_t, &_G.spawned_map, root.h, idx);
}

ARRAY_T(entity_t) *_get_spawned_array_by_idx(uint32_t idx) {
    return &ARRAY_AT(&_G.spawned_array, idx);
}

ARRAY_T(entity_t) *_get_spawned_array(entity_t entity) {
    uint32_t idx = MAP_GET(uint32_t, &_G.spawned_map, entity.h, UINT32_MAX);
    return &ARRAY_AT(&_G.spawned_array, idx);
}


void _destroy_spawned_array(entity_t entity) {
    uint32_t idx = MAP_GET(uint32_t, &_G.spawned_map, entity.h, UINT32_MAX);
    MAP_REMOVE(uint32_t, &_G.spawned_map, entity.h);

    ARRAY_T(entity_t) *array = &ARRAY_AT(&_G.spawned_array, idx);
    ARRAY_DESTROY(entity_t, array);
}

//==============================================================================
// Compiler private
//==============================================================================
#ifdef CETECH_CAN_COMPILE

static void preprocess(const char *filename,
                       yaml_node_t root,
                       struct compilator_api *capi) {
    yaml_node_t prefab_node = yaml_get_node(root, "prefab");

    if (yaml_is_valid(prefab_node)) {
        char prefab_file[256] = {0};
        char prefab_str[256] = {0};
        yaml_as_string(prefab_node, prefab_str, CETECH_ARRAY_LEN(prefab_str));
        snprintf(prefab_file, CETECH_ARRAY_LEN(prefab_file), "%s.entity",
                 prefab_str);

        capi->add_dependency(filename, prefab_file);

        char full_path[256] = {0};
        const char *source_dir = resource_api_v0.compiler_get_source_dir();
        path_v0.path_join(full_path, CETECH_ARRAY_LEN(full_path), source_dir,
                  prefab_file);

        struct vio *prefab_vio = vio_api_v0.from_file(full_path, VIO_OPEN_READ,
                                               memory_api_v0.main_allocator());

        char prefab_data[vio_api_v0.size(prefab_vio) + 1];
        memset(prefab_data, 0, vio_api_v0.size(prefab_vio) + 1);
        vio_api_v0.read(prefab_vio, prefab_data, sizeof(char),
                            vio_api_v0.size(prefab_vio));

        yaml_document_t h;
        yaml_node_t prefab_root = yaml_load_str(prefab_data, &h);

        preprocess(filename, prefab_root, capi);
        yaml_merge(root, prefab_root);

        vio_api_v0.close(prefab_vio);
    }
}

ARRAY_PROTOTYPE(yaml_node_t)

ARRAY_PROTOTYPE_N(struct array_yaml_node_t, array_yaml_node_t)

ARRAY_PROTOTYPE_N(struct array_uint32_t, array_uint32_t)

MAP_PROTOTYPE_N(struct array_yaml_node_t, array_yaml_node_t)

MAP_PROTOTYPE_N(struct array_uint32_t, array_uint32_t)

struct entity_compile_output {
    MAP_T(array_uint32_t) component_ent;
    MAP_T(uint32_t) entity_parent;
    MAP_T(array_yaml_node_t) component_body;
    ARRAY_T(uint64_t) component_type;
    uint32_t ent_counter;
};

struct foreach_children_data {
    int parent_ent;
    struct entity_compile_output *output;
};


static void compile_entitity(yaml_node_t rootNode,
                             int parent,
                             struct entity_compile_output *output);

void forach_children_clb(yaml_node_t key,
                         yaml_node_t value,
                         void *_data) {
    struct foreach_children_data *data = _data;

    compile_entitity(value, data->parent_ent, data->output);
}


struct foreach_componets_data {
    struct entity_compile_output *output;
    int ent_id;
};

void foreach_components_clb(yaml_node_t key,
                            yaml_node_t value,
                            void *_data) {
    char uid_str[256] = {0};
    char component_type_str[256] = {0};
    uint64_t cid;
    int contain_cid = 0;

    struct foreach_componets_data *data = _data;
    struct entity_compile_output *output = data->output;

    yaml_as_string(key, uid_str, CETECH_ARRAY_LEN(uid_str));

    yaml_node_t component_type_node = yaml_get_node(value, "component_type");
    yaml_as_string(component_type_node, component_type_str,
                   CETECH_ARRAY_LEN(component_type_str));

    cid = hash_api_v0.id64_from_str(component_type_str);

    for (int i = 0; i < ARRAY_SIZE(&output->component_type); ++i) {
        if (ARRAY_AT(&output->component_type, i) != cid) {
            continue;
        }

        contain_cid = 1;
    }

    if (!contain_cid) {
        ARRAY_PUSH_BACK(uint64_t, &output->component_type, cid);

        ARRAY_T(uint32_t) tmp_a = {0};
        ARRAY_INIT(uint32_t, &tmp_a, memory_api_v0.main_allocator());

        MAP_SET(array_uint32_t, &output->component_ent, cid, tmp_a);
    }

    if (!MAP_HAS(array_yaml_node_t, &output->component_body, cid)) {
        ARRAY_T(yaml_node_t) tmp_a = {0};
        ARRAY_INIT(yaml_node_t, &tmp_a, memory_api_v0.main_allocator());

        MAP_SET(array_yaml_node_t, &output->component_body, cid, tmp_a);

    }
    ARRAY_T(uint32_t) *tmp_a = MAP_GET_PTR(array_uint32_t,
                                           &output->component_ent,
                                           cid);
    ARRAY_PUSH_BACK(uint32_t, tmp_a, data->ent_id);

    ARRAY_T(yaml_node_t) *tmp_b = MAP_GET_PTR(array_yaml_node_t,
                                              &output->component_body, cid);
    ARRAY_PUSH_BACK(yaml_node_t, tmp_b, value);
}

static void compile_entitity(yaml_node_t rootNode,
                             int parent,
                             struct entity_compile_output *output) {

    uint32_t ent_id = output->ent_counter++;

    MAP_SET(uint32_t, &output->entity_parent, ent_id, parent);

    yaml_node_t components_node = yaml_get_node(rootNode, "components");
    CETECH_ASSERT("entity_system", yaml_is_valid(components_node));

    struct foreach_componets_data data = {
            .ent_id = ent_id,
            .output = output
    };

    yaml_node_foreach_dict(components_node, foreach_components_clb, &data);


    yaml_node_t children_node = yaml_get_node(rootNode, "children");
    if (yaml_is_valid(children_node)) {
        int parent_ent = ent_id;
        //output->ent_counter += 1;

        struct foreach_children_data data = {
                .parent_ent = parent_ent,
                .output = output
        };

        yaml_node_foreach_dict(children_node, forach_children_clb, &data);
    }
}


struct entity_compile_output *entity_compiler_create_output() {
    struct allocator *a = memory_api_v0.main_allocator();

    struct entity_compile_output *output =
    CETECH_ALLOCATE(a,
                    struct entity_compile_output,
                    1);
    output->ent_counter = 0;
    ARRAY_INIT(uint64_t, &output->component_type, a);
    MAP_INIT(array_uint32_t, &output->component_ent, a);
    MAP_INIT(uint32_t, &output->entity_parent, a);
    MAP_INIT(array_yaml_node_t, &output->component_body, a);

    return output;
}

void entity_compiler_destroy_output(struct entity_compile_output *output) {
    ARRAY_DESTROY(uint64_t, &output->component_type);
    MAP_DESTROY(uint32_t, &output->entity_parent);

    // clean inner array
    const MAP_ENTRY_T(array_uint32_t) *ce_it = MAP_BEGIN(array_uint32_t,
                                                         &output->component_ent);
    const MAP_ENTRY_T(array_uint32_t) *ce_end = MAP_END(array_uint32_t,
                                                        &output->component_ent);
    while (ce_it != ce_end) {
        ARRAY_DESTROY(uint32_t, (struct array_uint32_t *) &ce_it->value);
        ++ce_it;
    }
    MAP_DESTROY(array_uint32_t, &output->component_ent);

    // clean inner array
    const MAP_ENTRY_T(array_yaml_node_t) *cb_it = MAP_BEGIN(array_yaml_node_t,
                                                            &output->component_body);
    const MAP_ENTRY_T(array_yaml_node_t) *cb_end = MAP_END(array_yaml_node_t,
                                                           &output->component_body);
    while (cb_it != cb_end) {
        ARRAY_DESTROY(yaml_node_t, (struct array_yaml_node_t *) &cb_it->value);
        ++cb_it;
    }
    MAP_DESTROY(array_yaml_node_t, &output->component_body);

    struct allocator *a = memory_api_v0.main_allocator();
    CETECH_DEALLOCATE(a, output);
}

void entity_compiler_compile_entity(struct entity_compile_output *output,
                                    yaml_node_t root,
                                    const char *filename,
                                    struct compilator_api *compilator_api) {

    preprocess(filename, root, compilator_api);
    compile_entitity(root, UINT32_MAX, output);
}

uint32_t entity_compiler_ent_counter(struct entity_compile_output *output) {
    return output->ent_counter;
}

void entity_compiler_write_to_build(struct entity_compile_output *output,
                                    ARRAY_T(uint8_t) *build) {
    struct entity_resource res = {0};
    res.ent_count = (uint32_t) (output->ent_counter);
    res.comp_type_count = (uint32_t) ARRAY_SIZE(&output->component_type);

    ARRAY_PUSH(uint8_t, build, (uint8_t *) &res,
               sizeof(struct entity_resource));

    //write parents
    for (int i = 0; i < res.ent_count; ++i) {
        uint32_t id = MAP_GET(uint32_t, &output->entity_parent, i, UINT32_MAX);

        ARRAY_PUSH(uint8_t, build, (uint8_t *) &id, sizeof(id));
    }

    //write comp types
    ARRAY_PUSH(uint8_t, build, (uint8_t *) ARRAY_BEGIN(&output->component_type),
               sizeof(uint64_t) * ARRAY_SIZE(&output->component_type));

    //write comp data
    for (int j = 0; j < res.comp_type_count; ++j) {
        uint64_t cid = ARRAY_AT(&output->component_type, j);
        uint64_t id = cid;

        ARRAY_T(uint32_t) *ent_arr = MAP_GET_PTR(array_uint32_t,
                                                 &output->component_ent,
                                                 cid);

        struct component_data cdata = {
                .ent_count = ARRAY_SIZE(ent_arr)
        };

        ARRAY_T(yaml_node_t) *body = MAP_GET_PTR(array_yaml_node_t,
                                                 &output->component_body, cid);
        ARRAY_T(uint8_t) comp_data = {0};
        ARRAY_INIT(uint8_t, &comp_data, memory_api_v0.main_allocator());

        for (int i = 0; i < cdata.ent_count; ++i) {
            component_api_v0.component_compile(id, ARRAY_AT(body, i),
                                               &comp_data);
        }

        cdata.size = ARRAY_SIZE(&comp_data);

        ARRAY_PUSH(uint8_t, build, (uint8_t *) &cdata, sizeof(cdata));
        ARRAY_PUSH(uint8_t, build, (uint8_t *) ARRAY_BEGIN(ent_arr),
                   sizeof(uint32_t) * cdata.ent_count);
        ARRAY_PUSH(uint8_t, build, ARRAY_BEGIN(&comp_data),
                   sizeof(uint8_t) * ARRAY_SIZE(&comp_data));

        ARRAY_DESTROY(uint8_t, &comp_data);
    }
}

void entity_resource_compiler(yaml_node_t root,
                              const char *filename,
                              ARRAY_T(uint8_t) *build,
                              struct compilator_api *compilator_api) {
    struct entity_compile_output *output = entity_compiler_create_output();
    entity_compiler_compile_entity(output, root, filename, compilator_api);
    entity_compiler_write_to_build(output, build);

    entity_compiler_destroy_output(output);
}

int _entity_resource_compiler(const char *filename,
                              struct vio *source_vio,
                              struct vio *build_vio,
                              struct compilator_api *compilator_api) {
    char source_data[vio_api_v0.size(source_vio) + 1];
    memset(source_data, 0, vio_api_v0.size(source_vio) + 1);
    vio_api_v0.read(source_vio, source_data, sizeof(char),
             vio_api_v0.size(source_vio));

    yaml_document_t h;
    yaml_node_t root = yaml_load_str(source_data, &h);

    ARRAY_T(uint8_t) entity_data;
    ARRAY_INIT(uint8_t, &entity_data, memory_api_v0.main_allocator());

    entity_resource_compiler(root, filename, &entity_data, compilator_api);

    vio_api_v0.write(build_vio, &ARRAY_AT(&entity_data, 0), sizeof(uint8_t),
              ARRAY_SIZE(&entity_data));


    ARRAY_DESTROY(uint8_t, &entity_data);
    return 1;
}

#endif

//==============================================================================
// Resource
//==============================================================================

void *entity_resource_loader(struct vio *input,
                             struct allocator *allocator) {
    const int64_t size = vio_api_v0.size(input);
    char *data = CETECH_ALLOCATE(allocator, char, size);
    vio_api_v0.read(input, data, 1, size);

    return data;
}

void entity_resource_unloader(void *new_data,
                              struct allocator *allocator) {
    CETECH_DEALLOCATE(allocator, new_data);
}


void entity_resource_online(uint64_t name,
                            void *data) {
}

void entity_resource_offline(uint64_t name,
                             void *data) {
}

void *entity_resource_reloader(uint64_t name,
                               void *old_data,
                               void *new_data,
                               struct allocator *allocator) {
    entity_resource_offline(name, old_data);
    entity_resource_online(name, new_data);

    CETECH_DEALLOCATE(allocator, old_data);

    return new_data;
}

static const resource_callbacks_t entity_resource_callback = {
        .loader = entity_resource_loader,
        .unloader =entity_resource_unloader,
        .online =entity_resource_online,
        .offline =entity_resource_offline,
        .reloader = entity_resource_reloader
};



//==============================================================================
// Public interface
//==============================================================================

entity_t entity_manager_create() {
    return (entity_t) {.h = handler_api_v0.handler32_create(_G.entity_handler)};
}

void entity_manager_destroy(entity_t entity) {
    handler_api_v0.handler32_destroy(_G.entity_handler, entity.h);
}

int entity_manager_alive(entity_t entity) {
    return handler_api_v0.handler32_alive(_G.entity_handler, entity.h);
}


ARRAY_T(entity_t) *entity_spawn_from_resource(world_t world,
                                              void *resource) {
    struct entity_resource *res = resource;

    uint32_t idx = _new_spawned_array();
    ARRAY_T(entity_t) *spawned = _get_spawned_array_by_idx(idx);

    for (int j = 0; j < res->ent_count; ++j) {
        ARRAY_PUSH_BACK(entity_t, spawned,
                        entity_manager_create());
    }

    entity_t root = ARRAY_AT(spawned, 0);

    uint32_t *parents = entity_resource_parents(res);
    uint64_t *comp_types = entity_resource_comp_types(res);

    struct component_data *comp_data = entity_resource_comp_data(res);
    for (int i = 0; i < res->comp_type_count; ++i) {
        uint64_t type = comp_types[i];

        uint32_t *c_ent = component_data_ent(comp_data);
        char *c_data = component_data_data(comp_data);
        component_api_v0.component_spawn(world, type, &ARRAY_AT(spawned, 0),
                                         c_ent, parents, comp_data->ent_count,
                                         c_data);

        comp_data = (struct component_data *) (c_data + comp_data->size);
    }

    _map_spawned_array(root, idx);

    return spawned;
}

entity_t entity_spawn(world_t world,
                      uint64_t name) {
    void *res = resource_api_v0.get(_G.type, name);

    if (res == NULL) {
        log_api_v0.log_error("entity", "Could not spawn entity.");
        return (entity_t) {.h = 0};
    }

    ARRAY_T(entity_t) *spawned = entity_spawn_from_resource(world, res);

    return spawned->data[0];
}

void entity_destroy(world_t world,
                    entity_t *entity,
                    uint32_t count) {

    for (int i = 0; i < count; ++i) {
        ARRAY_T(entity_t) *spawned = _get_spawned_array(entity[i]);

        component_api_v0.component_destroy(world, spawned->data,
                                           spawned->size);

        _destroy_spawned_array(entity[i]);
    }

}

static void _init_api(struct api_v0* api){
    static struct entity_api_v0 _api = {0};
    _api.entity_manager_create = entity_manager_create;
    _api.entity_manager_destroy = entity_manager_destroy;
    _api.entity_manager_alive = entity_manager_alive;

    _api.spawn_from_resource = entity_spawn_from_resource;
    _api.spawn = entity_spawn;
    _api.destroy = entity_destroy;

#ifdef CETECH_CAN_COMPILE
    _api.compiler_create_output = entity_compiler_create_output;
    _api.compiler_destroy_output = entity_compiler_destroy_output;
    _api.compiler_compile_entity = entity_compiler_compile_entity;
    _api.compiler_ent_counter = entity_compiler_ent_counter;
    _api.compiler_write_to_build = entity_compiler_write_to_build;
    _api.resource_compiler = entity_resource_compiler;
#endif
    api->register_api("entity_api_v0", &_api);

}

static void _init( struct api_v0* api) {
    GET_API(api, memory_api_v0);
    GET_API(api, component_api_v0);
    GET_API(api, memory_api_v0);
    GET_API(api, resource_api_v0);
    GET_API(api, handler_api_v0);
    GET_API(api, path_v0);
    GET_API(api, vio_api_v0);
    GET_API(api, hash_api_v0);



    _G = (struct G) {0};

    _G.type = hash_api_v0.id64_from_str("entity");

    MAP_INIT(uint32_t, &_G.spawned_map, memory_api_v0.main_allocator());
    ARRAY_INIT(array_entity_t, &_G.spawned_array,
               memory_api_v0.main_allocator());

    resource_api_v0.register_type(_G.type, entity_resource_callback);

#ifdef CETECH_CAN_COMPILE
    resource_api_v0.compiler_register(_G.type, _entity_resource_compiler);
#endif

    _G.entity_handler = handler_api_v0.handler32gen_create(
            memory_api_v0.main_allocator());
}

static void _shutdown() {
    MAP_DESTROY(uint32_t, &_G.spawned_map);
    ARRAY_DESTROY(array_entity_t, &_G.spawned_array);

    handler_api_v0.handler32gen_destroy(_G.entity_handler);
    _G = (struct G) {0};
}


void *entity_get_module_api(int api) {
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