#pragma once

#include <stdint.h>
#include <stddef.h>

#define MAX_SERVERS 4
#define PROC_NAME_MAX_LEN 64  

const char *servers[MAX_SERVERS] = {
    "window_server.elf",
    "test_shell.elf",
    NULL
};

#define NUM_SERVERS (sizeof(servers) / sizeof(servers[0]))