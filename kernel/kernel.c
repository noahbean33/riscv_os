#include "kernel.h"
#include "uart.h"
#include "test.h"

void kernel_main()
{
    // clear screen, welcome message
    uart_cls();
    uart_printf("Novix RISC-V 64 OS, (c) NovixManiac, Version 0.0.1\n\n");
    uart_puts("Booting ...\n");

    // test getc
    getc_test();

    for (;;);
}