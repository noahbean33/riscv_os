#include "include/stdio.h"
#include "include/string.h"
#include "include/syscall.h"
#include "include/log.h"
#include "include/ipc.h"

void main()
{
    const char *msg = "draw: Hello GUI!";

    LOG("[shell] started...");
    puts("[shell] started...\n");
    LOG("[shell] sending message: %s @ %p", msg, msg);
    printf("[shell] sending message: %s @ %p\n", msg, msg);

    ipc_send(3, msg, strlen(msg));              // 3 = pid of window_server (temporarily hardcoded)
    LOG("Shell: draw commando verstuurd.");

    for(;;);
}