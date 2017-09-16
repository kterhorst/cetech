#ifndef COMMAND_SYSTEM_H
#define COMMAND_SYSTEM_H

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>


//==============================================================================
// Typedefs
//==============================================================================

typedef void (*ct_cmd_execute_t)(const struct ct_cmd *cmd,
                                 bool inverse);

//==============================================================================
// Structs
//==============================================================================

struct ct_cmd {
    uint64_t type;
    uint32_t size;
    char description[52];
};

//==============================================================================
// yDB
//==============================================================================

struct ct_ydb_cmd_s {
    const char *filename;
    uint64_t keys[32];
    uint32_t keys_count;
};


struct ct_ydb_cmd_bool_s {
    ct_cmd header;
    ct_ydb_cmd_s ydb;

    // VALUES
    bool new_value;
    bool old_value;
};

struct ct_ydb_cmd_float_s {
    ct_cmd header;
    ct_ydb_cmd_s ydb;

    // VALUES
    float new_value;
    float old_value;
};

struct ct_ydb_cmd_str_s {
    ct_cmd header;
    ct_ydb_cmd_s ydb;

    // VALUES
    char new_value[128];
    char old_value[128];
};

struct ct_ydb_cmd_vec3_s {
    ct_cmd header;
    ct_ydb_cmd_s ydb;

    // VALUES
    float new_value[3];
    float old_value[3];
};


//==============================================================================
// Api
//==============================================================================

//! Command system API V0
struct ct_cmd_system_a0 {
    void (*execute)(const struct ct_cmd *cmd);

    void (*register_cmd_execute)(uint64_t type,
                                 ct_cmd_execute_t execute);

    void (*undo)();

    void (*redo)();

    const char *(*undo_text)();

    const char *(*redo_text)();
};

#ifdef __cplusplus
}
#endif

#endif //COMMAND_SYSTEM_H
