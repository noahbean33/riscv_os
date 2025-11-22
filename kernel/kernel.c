#include "kernel.h"
#include "uart.h"
#include "trap.h"

void kernel_main()
{
    uart_cls();
    uart_printf("Novix RISC-V 64 OS, (c) NovixManiac, Version 0.0.1\n\n");
    uart_puts("Booting ...\n");

    // Initialize trap handling
    trap_init();

    // Force an illegal instruction to trigger an exception
    __asm__ volatile(".word 0xFFFFFFFF"); // Invalid opcode

    for (;;);
}