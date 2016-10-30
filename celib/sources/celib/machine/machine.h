#ifndef CELIB_MACHINE_H
#define CELIB_MACHINE_H

//==============================================================================
// Includes
//==============================================================================

#include "celib/math/types.h"
#include "types.h"
#include "celib/thread/types.h"

typedef int (*machine_part_init_t)();

typedef void (*machine_part_shutdown_t)();

typedef void (*machine_part_process_t)(struct eventstream *stream);

//==============================================================================
// Machine interface
//==============================================================================

int machine_init(int stage);

void machine_shutdown();

void machine_register_part(const char *name,
                           machine_part_init_t init,
                           machine_part_shutdown_t shutdown,
                           machine_part_process_t process);

void machine_process();

struct event_header *machine_event_begin();

struct event_header *machine_event_end();

struct event_header *machine_event_next(struct event_header *header);

int machine_gamepad_is_active(int gamepad);

void machine_gamepad_play_rumble(int gamepad,
                                 float strength,
                                 u32 length);

#endif //CELIB_MACHINE_H