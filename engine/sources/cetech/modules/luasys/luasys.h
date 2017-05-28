//! \defgroup Lua
//! Lua system
//! \{
#ifndef CETECH_LUA_TYPES_H
#define CETECH_LUA_TYPES_H

//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>
#include <stdint.h>

#include <cetech/core/math/math_types.h>

typedef struct lua_State lua_State;

typedef int (*lua_CFunction)(lua_State *L);

//==============================================================================
// Api
//==============================================================================

//! Lua system api V0
struct lua_api_v0 {

    //! Return number of elements in the stack, which is also the index of the top element.
    //! Notice that a negative index -x is equivalent to the positive index gettop - x + 1.
    //! \param l Lua state
    //! \return Number of elements
    int (*get_top)(lua_State *l);

    //! Removes the element at the given index, shifting down all elements on top of that position to fill in the gap
    //! \param l Lua state
    //! \param idx Elenebt idx
    void (*remove)(lua_State *l,
                   int idx);

    //! Remove n elements from stack
    //! \param l Lua state
    //! \param n Numbers of elements to remove
    void (*pop)(lua_State *l,
                int idx);

    //! Is value nil?
    //! \param l Lua state
    //! \param idx Element idx
    //! \return 1 if is nil else 0
    int (*is_nil)(lua_State *l,
                  int idx);

    //! Is value number?
    //! \param l Lua state
    //! \param idx Element idx
    //! \return 1 if is numberelse 0
    int (*is_number)(lua_State *l,
                     int idx);

    int (*is_vec2f)(lua_State *L,
                    int idx);

    int (*is_vec3f)(lua_State *L,
                    int idx);

    int (*is_vec4f)(lua_State *L,
                    int idx);

    int (*is_quat)(lua_State *L,
                   int idx);

    int (*is_mat44f)(lua_State *L,
                     int idx);

    //! Get element type
    //! \param l Lua state
    //! \param idx Element idx
    //! \return Element type
    int (*value_type)(lua_State *l,
                      int idx);

    //! Push nil
    //! \param l Lua state
    void (*push_nil)(lua_State *l);

    //! Push uint64_t
    //! \param l Lua state
    //! \param value Value
    void (*push_uint64_t)(lua_State *l,
                          uint64_t value);

    //! Push handler
    //! \param l
    //! \param value
    void (*push_handler)(lua_State *l,
                         uint32_t value);

    //! Push int
    //! \param l
    //! \param value Value
    void (*push_int)(lua_State *l,
                     int value);

    //! Push bool
    //! \param l
    //! \param value Value
    void (*push_bool)(lua_State *l,
                      int value);

    //! Push float
    //! \param l
    //! \param value Value
    void (*push_float)(lua_State *l,
                       float value);

    //! Push string
    //! \param l
    //! \param value Value
    void (*push_string)(lua_State *l,
                        const char *value);

    //! Get element value as bool
    //! \param l
    //! \param i Element idx
    //! \return Bool value
    int (*to_bool)(lua_State *l,
                   int i);

    //! Get element value as int
    //! \param l
    //! \param i Element idx
    //! \return Int value
    int (*to_int)(lua_State *l,
                  int i);

    //! Get element value as u64
    //! \param l
    //! \param i Element idx
    //! \return sInt value
    uint64_t (*to_u64)(lua_State *l,
                       int i);

    //! Get element value as float
    //! \param l
    //! \param i Element idx
    //! \return Float value
    float (*to_float)(lua_State *l,
                      int i);

    //! Get element value as hadler
    //! \param l
    //! \param i Element idx
    //! \return Handler value
    uint32_t (*to_handler)(lua_State *l,
                           int i);

    //! Get element value as string
    //! \param l
    //! \param i Element idx
    //! \return String value
    const char *(*to_string)(lua_State *,
                             int i);

    //! Get element value as string and set len
    //! \param l
    //! \param i Element idx
    //! \param len Write string len to this variable
    //! \return String value
    const char *(*to_string_l)(lua_State *l,
                               int i,
                               size_t *len);

    //! Get element value as vec2f
    //! \param l
    //! \param i Element idx
    //! \return Vec2f
    vec2f_t *(*to_vec2f)(lua_State *l,
                         int i);

    //! Get element value as vec3f
    //! \param l
    //! \param i Element idx
    //! \return Vec3f
    vec3f_t *(*to_vec3f)(lua_State *l,
                         int i);

    //! Get element value as vec4f
    //! \param l
    //! \param i Element idx
    //! \return Vec4f
    vec4f_t *(*to_vec4f)(lua_State *l,
                         int i);

    //! Get element value as mat44f
    //! \param l
    //! \param i Element idx
    //! \return Mat44f
    mat44f_t *(*to_mat44f)(lua_State *l,
                           int i);

    //! Get element value as quat
    //! \param l
    //! \param i Element idx
    //! \return Quatf
    quatf_t *(*to_quat)(lua_State *l,
                        int i);

    //! Push vec2f
    //! \param l
    //! \param v Value
    void (*push_vec2f)(lua_State *l,
                       vec2f_t v);

    //! Push vec3f
    //! \param l
    //! \param v Value
    void (*push_vec3f)(lua_State *l,
                       vec3f_t v);

    //! Push vec4f
    //! \param l
    //! \param v Value
    void (*push_vec4f)(lua_State *l,
                       vec4f_t v);

    //! Push mat44f
    //! \param l
    //! \param v Value
    void (*push_mat44f)(lua_State *l,
                        mat44f_t v);

    //! Push quatf
    //! \param l
    //! \param v Value
    void (*push_quat)(lua_State *l,
                      quatf_t v);

    //! Execute lua code
    //! \param str Lua code
    //! \return 0 if execute fail else 1
    int (*execute_string)(const char *str);

    //! Execute resource
    //! \param name Resource name
    void (*execute_resource)(uint64_t name);

    //! Add module function
    //! \param module Module name
    //! \param name Function name
    //! \param func Function
    void (*add_module_function)(const char *module,
                                const char *name,
                                const lua_CFunction func);

    //! Get game callbacks
    //! \return Lua game callbacks
    const struct game_callbacks *(*get_game_callbacks)();

    //! Execute boot script
    //! \param Boot script name
    void (*execute_boot_script)(uint64_t name);

    //! Call global function
    //! \example lua_api_v0.call_global("print", "if", 1, 2.0f);
    //! \param func Function name
    //! \param args Function args format string ('i' = int, 'f' = float)
    //! \param ... Args value
    void (*call_global)(const char *func,
                        const char *args,
                        ...);
};

/// REMOVE

int luasys_get_top(lua_State *l);

void luasys_remove(lua_State *l,
                   int idx);

void luasys_pop(lua_State *l,
                int idx);

int luasys_is_nil(lua_State *l,
                  int idx);

int luasys_is_number(lua_State *l,
                     int idx);

int luasys_value_type(lua_State *l,
                      int idx);

void luasys_push_nil(lua_State *l);

void luasys_push_uint64_t(lua_State *l,
                          uint64_t value);

void luasys_push_handler(lua_State *l,
                         uint32_t value);

void luasys_push_int(lua_State *l,
                     int value);

void luasys_push_uint32_t(lua_State *l,
                          uint32_t value);

void luasys_push_bool(lua_State *l,
                      int value);

void luasys_push_float(lua_State *l,
                       float value);

void luasys_push_string(lua_State *l,
                        const char *value);

int luasys_to_bool(lua_State *l,
                   int i);

int luasys_to_int(lua_State *l,
                  int i);

uint32_t luasys_to_uin32_t(lua_State *l,
                           int i);


float luasys_to_float(lua_State *l,
                      int i);

uint32_t luasys_to_handler(lua_State *l,
                           int i);

const char *luasys_to_string(lua_State *,
                             int i);

const char *luasys_to_string_l(lua_State *,
                               int,
                               size_t *);

vec2f_t *luasys_to_vec2f(lua_State *l,
                         int i);

vec3f_t *luasys_to_vec3f(lua_State *l,
                         int i);

vec4f_t *luasys_to_vec4f(lua_State *l,
                         int i);

mat44f_t *luasys_to_mat44f(lua_State *l,
                           int i);

quatf_t *luasys_to_quat(lua_State *l,
                        int i);

void luasys_push_vec2f(lua_State *l,
                       vec2f_t v);

void luasys_push_vec3f(lua_State *l,
                       vec3f_t v);

void luasys_push_vec4f(lua_State *l,
                       vec4f_t v);

void luasys_push_mat44f(lua_State *l,
                        mat44f_t v);


void luasys_push_quat(lua_State *l,
                      quatf_t v);

int luasys_execute_string(const char *str);

void luasys_add_module_function(const char *module,
                                const char *name,
                                const lua_CFunction func);

void luasys_add_module_constructor(const char *module,
                                   const lua_CFunction func);

void luasys_execute_resource(uint64_t name);

const struct game_callbacks *luasys_get_game_callbacks();

void luasys_execute_boot_script(uint64_t name);

void luasys_call_global(const char *func,
                        const char *args,
                        ...);


#endif //CETECH_LUA_TYPES_H
//! \}