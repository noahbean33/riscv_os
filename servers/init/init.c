#include "include/init.h"
#include "../../user/include/types.h"
#include "../../user/include/stdio.h"
#include "../../user/include/syscall.h"
#include "../../user/include/log.h"
#include "../../user/include/common.h"

static int restart_count = 0;

void main(void) {
    LOG("Starting system services...");

    // We give each server exactly 1 argument: its own name (without .elf)
    char *argv_init[2];
    int   argc_init = 1;
    argv_init[1] = NULL;  // argv array must be NULL-terminated

    for (int i = 0; i < (int)NUM_SERVERS; ++i) {
        const char *elf_path = servers[i];
        if (!elf_path) continue;

        // extract basename without .elf from servers[i]
        static char namebuf[PROC_NAME_MAX_LEN];
        strip_elf_extension(elf_path, namebuf, sizeof(namebuf));
        argv_init[0] = namebuf;

        pid_t pid = fork();
        if (pid == 0) {
            // Child: run the server with our local argv_init
            exec(elf_path, argv_init, argc_init);
            LOG("[init] exec failed for %s", elf_path);
            exit(1);
        }
        LOG("started %s as pid %d", elf_path, pid);
    }

    // and keep restarting if one crashes
    for (;;) {
        int status;
        pid_t dead = wait(&status);
        restart_count += 1;
        printf("[init] process %d exited with code %d — RESTARTING %d\n",
               dead, status, restart_count);

        for (int i = 0; i < (int)NUM_SERVERS; ++i) {
            const char *elf_path = servers[i];
            if (!elf_path) continue;

            static char namebuf[PROC_NAME_MAX_LEN];
            strip_elf_extension(elf_path, namebuf, sizeof(namebuf));
            argv_init[0] = namebuf;

            pid_t pid = fork();
            if (pid == 0) {
                exec(elf_path, argv_init, argc_init);
                exit(1);
            }
            printf("[init] restarted %s as pid %d\n", elf_path, pid);
        }
    }
}