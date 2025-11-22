#pragma once 

#include "kernel.h"

#define assert(cond)                                   \
    do {                                               \
        if (!(cond)) {                                 \
            uart_printf("ASSERT FAILED: %s\n", #cond); \
            PANIC("Assertion failed");                 \
        }                                              \
    } while (0)

// C11‑style compile‑time assert
#define STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)