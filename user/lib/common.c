#include "../include/common.h"
#include "../include/string.h"

static const char digits[] = "0123456789abcdef";

void itoa_unsigned(unsigned int value, char *str, int base) {
    char buf[33];
    int i = 0;
    if (base < 2 || base > 16) {
        str[0] = '\0';
        return;
    }
    do {
        buf[i++] = digits[value % base];
        value /= base;
    } while (value > 0);

    // Turn around
    while (i > 0) {
        *str++ = buf[--i];
    }
    *str = '\0';
}

void itoa_signed(int value, char *str, int base) {
    if (base < 2 || base > 16) {
        str[0] = '\0';
        return;
    }
    if (value < 0 && base == 10) {
        *str++ = '-';
        itoa_unsigned((unsigned int)(-value), str, base);
    } else {
        itoa_unsigned((unsigned int)value, str, base);
    }
}

int kvnprintf(char *buf, size_t size, const char *fmt, va_list args) {
    size_t i = 0;

    while (*fmt && i < size - 1) {
        if (*fmt == '%' && *(fmt + 1)) {
            char spec = *(fmt + 1);
            fmt += 2;

            switch (spec) {
                case 's': {
                    const char *s = va_arg(args, const char *);
                    while (s && *s && i < size - 1) buf[i++] = *s++;
                    break;
                }
                case 'd': {
                    int val = va_arg(args, int);
                    char tmp[33];
                    itoa_signed(val, tmp, 10);
                    const char *s = tmp;
                    while (*s && i < size - 1) buf[i++] = *s++;
                    break;
                }
                case 'u': {
                    unsigned int val = va_arg(args, unsigned int);
                    char tmp[33];
                    itoa_unsigned(val, tmp, 10);
                    const char *s = tmp;
                    while (*s && i < size - 1) buf[i++] = *s++;
                    break;
                }
                case 'x': {
                    unsigned int val = va_arg(args, unsigned int);
                    char tmp[33];
                    itoa_unsigned(val, tmp, 16);
                    const char *s = tmp;
                    while (*s && i < size - 1) buf[i++] = *s++;
                    break;
                }
                case 'p': {
                    void *ptr = va_arg(args, void *);
                    unsigned long addr = (unsigned long)ptr;
                    char tmp[33];
                    buf[i++] = '0';
                    if (i < size - 1) buf[i++] = 'x';
                    itoa_unsigned((unsigned int)addr, tmp, 16);
                    const char *s = tmp;
                    while (*s && i < size - 1) buf[i++] = *s++;
                    break;
                }
                case 'c': {
                    int ch = va_arg(args, int);
                    buf[i++] = (char)ch;
                    break;
                }
                case '%': {
                    buf[i++] = '%';
                    break;
                }
                default: {
                    buf[i++] = '%';
                    if (i < size - 1) buf[i++] = spec;
                    break;
                }
            }
        } else {
            buf[i++] = *fmt++;
        }
    }

    buf[i] = '\0';
    return (int)i;
}

int ksnprintf(char *buf, size_t size, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int ret = kvnprintf(buf, size, fmt, args);
    va_end(args);
    return ret;
}

void strip_elf_extension(const char *progname, char *out_name, size_t maxlen) {
    if (!progname || !out_name || maxlen == 0) return;

    strncpy(out_name, progname, maxlen - 1);
    out_name[maxlen - 1] = '\0';  // null-terminated

    char *dot = strstr(out_name, ".elf");
    if (dot && strcmp(dot, ".elf") == 0) {
        *dot = '\0';  // truncate at ".elf"
    }
}