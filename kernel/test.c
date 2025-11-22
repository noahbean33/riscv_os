#include "uart.h"

void getc_test(void)
{
    uart_printf("Kernel booted. Type a key: ");

    int c;
    while ((c = uart_getc()) == -1) {
    //Wait until a key is pressed
    }

    uart_putc(c); // echo key
    uart_printf("\nReady with uart_getc() test\n");
}
