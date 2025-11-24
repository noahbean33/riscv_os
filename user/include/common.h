#pragma once

#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>

#define align_up(value, align)   __builtin_align_up(value, align)
#define is_aligned(value, align) __builtin_is_aligned(value, align)

int ksnprintf(char *buf, size_t size, const char *fmt, ...);
int kvnprintf(char *buf, size_t size, const char *fmt, va_list args);
void itoa(int value, char *str, int base);
void strip_elf_extension(const char *progname, char *out_name, size_t maxlen);