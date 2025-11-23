#pragma once
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>

#include "types.h"


#define align_up(value, align)   __builtin_align_up(value, align)
#define is_aligned(value, align) __builtin_is_aligned(value, align)

int ksnprintf(char *buf, size_t size, const char *fmt, ...);
int kvnprintf(char *buf, size_t size, const char *fmt, va_list args);
void itoa(int value, char *str, int base);