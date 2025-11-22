#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include "uart.h"

void kernel_main(void);

#define PANIC(fmt, ...)                                                             \
    do  {                                                                           \
        uart_puts("\n[!!! PANIC !!!] ");                                            \
        uart_printf("FILE : %s:%d : " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
        uart_puts("System halted.\n");                                              \
        while (1) {}                                                                \
    } while (0)