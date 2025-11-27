#include "include/window_server.h"
#include "../../user/include/types.h"
#include "../../user/include/stdio.h"
#include "../../user/include/string.h"
#include "../../user/include/ipc.h"
#include "../../user/include/syscall.h"
#include "../../user/include/log.h"

int main() {
    LOG("[window_server] Ready. Waiting for draw commands...");

    while (1) {
        char buffer[BUF_SIZE];
        pid_t sender;

        LOG("[window_server] Start recieving...");
        puts("[window_server] Start recieving...\n");

        int size = ipc_recv(buffer, BUF_SIZE, &sender);

        LOG("[window_server] Ready recieving...");
        printf("[window_server] Ready recieving...\n");

        if (size > 0) {
            buffer[size] = '\0';
            LOG("[window_server] Got message: %s from pid %d", buffer, sender);
            printf("[window_server] Got message: %s from pid %d\n", buffer, sender);

            if (strncmp(buffer, "draw", 4) == 0) {
                LOG("[window_server] Drawing: %s", buffer + 5);
                printf("[window_server] Drawing: %s\n", buffer + 5);
            } else {
                LOG("[window_server] Unknown command.");
                printf("[window_server] Unknown command.\n");

            }
        }
    }

    return 0;
}