#include "kernel.h"

#define UART0_BASE 0x10000000UL
#define UART0_THR (*(volatile unsigned char *)(UART0_BASE + 0x00))
#define UART0_LSR (*(volatile unsigned char *)(UART0_BASE + 0x05))

static inline int uart_is_writable() {
    return UART0_LSR & 0x20;
}

void putc(char c) {
    while (!uart_is_writable());
    UART0_THR = c;
}

void puts(const char *s) {
    while (*s) {
        if (*s == '\n') putc('\r');
            putc(*s++);
    }
}

void kernel_main() {
    puts("hello world!\n");
    for (;;);
}
