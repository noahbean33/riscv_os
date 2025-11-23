#include "kernel.h"
#include "uart.h"
#include "trap.h"
#include "bss.h"
#include "stack.h"
#include "ram.h"
#include "heap.h"
#include "test.h"

void kernel_main(void)
{
    // clear bss
    bss_init();

    // clear screen, welcome message
    uart_cls();
    uart_printf("Novix OS (64-bits), (c) Novix, Version 0.0.1\n\n");
    uart_puts("Booting ...\n");

    // initialize trap_vector
    trap_init();

    // initialize stack, ram & heap
    stack_init();
    ram_init();
    heap_init();

    // test malloc & free
    heap_test();

    for (;;);
}