#pragma once

#include <stdarg.h>
#include <stdint.h>

#define ANSI_RESET       "\x1b[0m"
#define ANSI_RED         "\x1b[31m"
#define ANSI_GREEN       "\x1b[32m"
#define ANSI_YELLOW      "\x1b[33m"
#define ANSI_BLUE        "\x1b[34m"
#define ANSI_MAGENTA     "\x1b[35m"
#define ANSI_CYAN        "\x1b[36m"
#define ANSI_WHITE       "\x1b[37m"
#define ANSI_BOLD        "\x1b[1m"
#define ANSI_UNDERLINE   "\x1b[4m"

void uart_putc(char c);
long uart_getc(void);
void uart_puts(const char *s);
void uart_printf(const char *fmt, ...);
void uart_cls();
void uart_set_color(const char *ansi_code);

void uart_puthex8(uint8_t val);
void uart_puthex8_prefixed(uint8_t val);
void uart_puthex16(uint16_t val);
void uart_puthex16_prefixed(uint16_t val);
void uart_puthex32(uint32_t val);
void uart_puthex32_prefixed(uint32_t val);
void uart_puthex64(uint64_t val);
void uart_puthex64_prefixed(uint64_t val);