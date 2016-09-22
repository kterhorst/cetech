
#include <engine/core/world_system.h>
#include <celib/stringid/stringid.h>
#include "engine/luasys/lua_system.h"

#define API_NAME "World"

static int _world_create(lua_State *l) {
    world_t world = world_create();
    luasys_push_handler(l, world.h);
    return 1;
}

static int _world_destroy(lua_State *l) {
    world_t world = {.h = luasys_to_handler(l, 1)};
    world_destroy(world);
    return 0;
}

static int _load_level(lua_State *l) {
    world_t world = {.h = luasys_to_handler(l, 1)};
    stringid64_t name = stringid64_from_string(luasys_to_string(l, 2));

    world_load_level(world, name);
    return 0;
}


void _register_lua_world_api() {
    luasys_add_module_function(API_NAME, "create", _world_create);
    luasys_add_module_function(API_NAME, "destroy", _world_destroy);

    luasys_add_module_function(API_NAME, "load_level", _load_level);
}