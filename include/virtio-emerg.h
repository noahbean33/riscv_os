#pragma once

#include <stdint.h>
#include <stdarg.h>

void virtio_emerg_init(volatile uint32_t *regs);
void virtio_emerg_log(const char *level, const char *unused_color, const char *fmt, ...);
void virtio_emerg_log_ext(const char *source, int pid, const char *level, const char *label, const char *fmt, ...);
void virtio_emerg_log_va(const char *level, const char *unused_color, const char *fmt, va_list args);
