#include "../include/syscall.h"
#include "../include/types.h"
#include "../include/stdio.h"
#include "../include/log.h"

int ipc_send(pid_t pid, const void *buf, size_t size) {
    return syscall(SYS_IPC_SEND, (intptr_t)pid, (intptr_t)buf, (intptr_t)size);
}


int ipc_recv(void *buf, size_t max_size, pid_t *sender) {
    int ret = 0;;
    
    do {
        LOG("[ipc_recv] start syscall SYS_IPC_RECV");
        ret = syscall(SYS_IPC_RECV,
                      (intptr_t)buf,
                      (intptr_t)max_size,
                      (intptr_t)sender);
        LOG("[ipc_recv] syscall SYS_IPC_RECV done! ret = %d", ret);
       
    } while (ret < 0);
    LOG("[ipc_recv] received  msg : %s, sender = %d, len = %d", (const char*)buf, (intptr_t)sender, ret);
    return ret;
}