#pragma once

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

void ulog(const char *fmt, ...);

// Simpelere macro
#define LOG(...) ulog(__VA_ARGS__)