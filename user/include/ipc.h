#pragma once
#include <stddef.h>

int ipc_send(int pid, const void *buf, size_t size);
int ipc_recv(void *buf, size_t max_size, int *sender);