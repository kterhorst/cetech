#include <stdio.h>
#include <unistd.h>

#include "celib/macros.h"

#include <cetech/kernel/api_system.h>
#include <cetech/kernel/process.h>
#include <cetech/kernel/module.h>
#include "cetech/kernel/log.h"

CETECH_DECL_API(ct_log_a0);

int exec(const char *argv) {
#if defined(CETECH_LINUX) || defined(CETECH_DARWIN)
    char output[4096];

    ct_log_a0.debug("os_sdl", "exec %s", argv);

    FILE *fp = popen(argv, "r");
    if (fp == NULL)
        return -1;

    while (fgets(output, CETECH_ARRAY_LEN(output), fp) != NULL) {
        printf("%s", output);
    }

    int status = pclose(fp);

    if (status == -1) {
        ct_log_a0.error("os_sdl", "output %s", output);
    }

    return status;
#else
#error "Implement this"
#endif
}

static ct_process_a0 process_api = {
        .exec = exec
};


CETECH_MODULE_DEF(
        process,
        {
            CETECH_GET_API(api, ct_log_a0);

        },
        {
            CEL_UNUSED(reload);
            api->register_api("ct_process_a0", &process_api);
        },
        {
            CEL_UNUSED(reload);
            CEL_UNUSED(api);
        }
)
