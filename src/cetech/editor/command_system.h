#ifndef COMMAND_SYSTEM_H
#define COMMAND_SYSTEM_H

//==============================================================================
// Includes
//==============================================================================
#include <stdint.h>
#include <stddef.h>

#include <celib/module.inl>

//==============================================================================
// Typedefs
//==============================================================================

//==============================================================================
// Structs
//==============================================================================

struct ct_cmd {
    uint64_t type;
    uint32_t size;
};

//==============================================================================
// yDB CDB
//==============================================================================

struct ct_cdb_cmd_s {
    struct ct_cmd header;

    uint64_t obj;
    uint64_t prop;

    // VALUES
    union {
        struct {
            float new_value[3];
            float old_value[3];
        } vec3;

        struct {
            float new_value;
            float old_value;
        } f;

        struct {
            uint64_t new_value;
            uint64_t old_value;
        } u64;

        struct {
            char new_value[128];
            char old_value[128];
        } str;

        struct {
            bool new_value;
            bool old_value;
        } b;
    };
};


typedef void (*ct_cmd_execute_t)(const struct ct_cmd *cmd,
                                 bool inverse);

typedef void (*ct_cmd_description_t)(char *buffer,
                                     uint32_t buffer_size,
                                     const struct ct_cmd *cmd,
                                     bool inverse);

struct ct_cmd_fce {
    ct_cmd_execute_t execute;
    ct_cmd_description_t description;
};

//==============================================================================
// Api
//==============================================================================

struct ct_cmd_system_a0 {
    void (*execute)(const struct ct_cmd *cmd);

    void (*register_cmd_execute)(uint64_t type,
                                 struct ct_cmd_fce fce);

    void (*undo)();

    void (*redo)();

    void (*undo_text)(char *buffer,
                      uint32_t buffer_size);

    void (*redo_text)(char *buffer,
                      uint32_t buffer_size);

    void (*command_text)(char *buffer,
                         uint32_t buffer_size,
                         uint32_t idx);

    uint32_t (*command_count)();

    uint32_t (*curent_idx)();

    void (*goto_idx)(uint32_t idx);
};

CE_MODULE(ct_cmd_system_a0);

#endif //COMMAND_SYSTEM_H
