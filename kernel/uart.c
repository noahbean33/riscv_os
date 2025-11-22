#include "common.h"
#include "uart.h"
#include "sbi.h"

long uart_getc(void) {
    struct sbiret ret = sbi_call(0, 0, 0, 0, 0, 0, 0, 2);
    return ret.error;
}

void uart_putc(char ch) {
    sbi_call(ch, 0, 0, 0, 0, 0, 0, 1);
}


void uart_puts(const char *s) {
    while (*s) {
        uart_putc(*s++);
    }
}

static const char HEX_DIGITS[] = "0123456789ABCDEF";

// Print 8-bit hex (2 chars)
void uart_puthex8(uint8_t val) {
    uart_putc(HEX_DIGITS[(val >> 4) & 0xF]);
    uart_putc(HEX_DIGITS[val & 0xF]);
}

// Optioneel: prefix met "0x"
void uart_puthex8_prefixed(uint8_t val) {
    uart_puts("0x");
    uart_puthex8(val);
}

// Print 16-bit hex (4 chars)
void uart_puthex16(uint16_t val) {
    uart_puthex8((val >> 8) & 0xFF);
    uart_puthex8(val & 0xFF);
}

void uart_puthex16_prefixed(uint16_t val) {
    uart_puts("0x");
    uart_puthex16(val);
}

// Print 32-bit hex (8 chars)
void uart_puthex32(uint32_t val) {
    uart_puthex16((val >> 16) & 0xFFFF);
    uart_puthex16(val & 0xFFFF);
}

void uart_puthex32_prefixed(uint32_t val) {
    uart_puts("0x");
    uart_puthex32(val);
}

// Print 64-bit hex (16 chars)
void uart_puthex64(uint64_t val) {
    uart_puthex32((uint32_t)(val >> 32));
    uart_puthex32((uint32_t)(val & 0xFFFFFFFF));
}

void uart_puthex64_prefixed(uint64_t val) {
    uart_puts("0x");
    uart_puthex64(val);
}

void uart_put_udec(uint64_t value) {
    char buf[21];
    int i = 20;
    buf[i--] = '\0';

    if (value == 0) {
        uart_putc('0');
        return;
    }

    while (value && i >= 0) {
        buf[i--] = '0' + (value % 10);
        value /= 10;
    }

    uart_puts(&buf[i + 1]);
}

void uart_put_dec(int64_t value) {
    if (value < 0) {
        uart_putc('-');
        uart_put_udec(-value);
    } else {
        uart_put_udec(value);
    }
}

void uart_puthex(uint64_t value, int width) {
    char hex_digits[] = "0123456789abcdef";
    char buf[17];
    int i = 16;

    buf[i--] = '\0';
    if (value == 0) {
        buf[i--] = '0';
    }

    while (value && i >= 0) {
        buf[i--] = hex_digits[value & 0xf];
        value >>= 4;
    }

    int len = 16 - i - 1;
    for (int j = 0; j < width - len; j++) uart_putc('0');
    uart_puts(&buf[i + 1]);
}

void uart_putpad(uint64_t val, int base, int width, int zero_pad, int signed_val) {
    char buf[32];
    int i = 31;
    buf[i--] = '\0';

    int negative = 0;
    if (signed_val && (int64_t)val < 0) {
        negative = 1;
        val = (uint64_t)(-(int64_t)val);
    }

    if (val == 0) {
        buf[i--] = '0';
    } else {
        while (val && i >= 0) {
            int digit = val % base;
            buf[i--] = (digit < 10) ? '0' + digit : 'a' + digit - 10;
            val /= base;
        }
    }

    int len = 31 - i;
    if (negative) {
        uart_putc('-');
    }

    for (int j = len; j < width; j++) {
        uart_putc(zero_pad ? '0' : ' ');
    }

    uart_puts(&buf[i + 1]);
}

void uart_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;

            // Parse flags
            int zero_pad = 0;
            if (*fmt == '0') {
                zero_pad = 1;
                fmt++;
            }

            // Parse field width
            int width = 0;
            while (*fmt >= '0' && *fmt <= '9') {
                width = width * 10 + (*fmt - '0');
                fmt++;
            }

            // Length modifier
            int length = 0; // 0 = default, 1 = l, 2 = ll
            if (*fmt == 'l') {
                fmt++;
                length = 1;
                if (*fmt == 'l') {
                    fmt++;
                    length = 2;
                }
            }

            switch (*fmt) {
                case 'd':
                    if (length == 2)
                        uart_putpad(va_arg(args, long long), 10, width, zero_pad, 1);
                    else if (length == 1)
                        uart_putpad(va_arg(args, long), 10, width, zero_pad, 1);
                    else
                        uart_putpad(va_arg(args, int), 10, width, zero_pad, 1);
                    break;
                case 'u':
                    if (length == 2)
                        uart_putpad(va_arg(args, unsigned long long), 10, width, zero_pad, 0);
                    else if (length == 1)
                        uart_putpad(va_arg(args, unsigned long), 10, width, zero_pad, 0);
                    else
                        uart_putpad(va_arg(args, unsigned int), 10, width, zero_pad, 0);
                    break;
                case 'x':
                    if (length == 2 || length == 1)
                        uart_puthex(va_arg(args, uint64_t), width);
                    else
                        uart_puthex(va_arg(args, uint32_t), width);
                    break;
                case 'c':
                    uart_putc(va_arg(args, int));
                    break;
                case 's': {
                    char *s = va_arg(args, char*);
                    uart_puts(s ? s : "(null)");
                    break;
                }
                case 'p':
                    uart_puts("0x");
                    uart_puthex((uintptr_t)va_arg(args, void *), sizeof(void*) * 2);
                    break;
                case '%':
                    uart_putc('%');
                    break;
                default:
                    uart_putc('%');
                    uart_putc(*fmt);
                    break;
            }
        } else {
            uart_putc(*fmt);
        }
        fmt++;
    }

    va_end(args);
}

void uart_cls() {
    uart_puts("\x1b[2J"); // Clear screen
    uart_puts("\x1b[H");  // Move cursor to home (1;1)
}

void uart_set_color(const char *ansi_code) {
    uart_puts(ansi_code);
}