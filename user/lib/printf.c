#include "../include/syscall.h"
#include "../include/string.h"


void putc(char ch) {
    syscall(SYS_PUTCHAR, ch, 0, 0);
}

void puts(const char *buf){
    write(1, buf, strlen(buf));
}

void puthex(uint64_t value, int width) {
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
    for (int j = 0; j < width - len; j++) putc('0');
    puts(&buf[i + 1]);
}


void putpad(uint64_t val, int base, int width, int zero_pad, int signed_val) {
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
        putc('-');
    }

    for (int j = len; j < width; j++) {
        putc(zero_pad ? '0' : ' ');
    }

    puts(&buf[i + 1]);
}

void printf(const char *fmt, ...) {
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
                        putpad(va_arg(args, long long), 10, width, zero_pad, 1);
                    else if (length == 1)
                        putpad(va_arg(args, long), 10, width, zero_pad, 1);
                    else
                        putpad(va_arg(args, int), 10, width, zero_pad, 1);
                    break;
                case 'u':
                    if (length == 2)
                        putpad(va_arg(args, unsigned long long), 10, width, zero_pad, 0);
                    else if (length == 1)
                        putpad(va_arg(args, unsigned long), 10, width, zero_pad, 0);
                    else
                        putpad(va_arg(args, unsigned int), 10, width, zero_pad, 0);
                    break;
                case 'x':
                    if (length == 2 || length == 1)
                        puthex(va_arg(args, uint64_t), width);
                    else
                        puthex(va_arg(args, uint32_t), width);
                    break;
                case 'c':
                    putc(va_arg(args, int));
                    break;
                case 's': {
                    char *s = va_arg(args, char*);
                    puts(s ? s : "(null)");
                    break;
                }
                case 'p':
                    puts("0x");
                    puthex((uintptr_t)va_arg(args, void *), sizeof(void*) * 2);
                    break;
                case '%':
                    putc('%');
                    break;
                default:
                    putc('%');
                    putc(*fmt);
                    break;
            }
        } else {
            putc(*fmt);
        }
        fmt++;
    }

    va_end(args);
}