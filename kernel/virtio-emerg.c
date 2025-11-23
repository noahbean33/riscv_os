#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include "virtio-console.h"
#include "common.h"
#include "string.h"
#include "uart.h"

static volatile uint32_t *emerg_ptr = 0;

void virtio_emerg_init(volatile uint32_t *regs) {
    emerg_ptr = (volatile uint32_t *)(
        (uintptr_t)regs + VIRTIO_MMIO_DEVICE_CONFIG +
        offsetof(struct virtio_console_config, emerg_wr)
    );
    uart_printf("[virtio_emerge_init] Initialized at 0x%x\n", regs);
}

void emerg_putc(char c) {
    if (emerg_ptr) {
        *emerg_ptr = (uint32_t)c;
    }
}

void emerg_puts(const char *s) {
    while (*s) {
        emerg_putc(*s++);
    }
}

void emerg_printf(const char *fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    kvnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    emerg_puts(buf);
}

void virtio_emerg_log(const char *level, const char *unused_color, const char *fmt, ...) {
    (void)unused_color;  // do not use color, prevents compiler warning

    char buf[512];
    int pos = 0;
    pos += ksnprintf(buf + pos, sizeof(buf) - pos, "[%s] ", level);

    va_list args;
    va_start(args, fmt);
    pos += kvnprintf(buf + pos, sizeof(buf) - pos, fmt, args);
    va_end(args);

    pos += ksnprintf(buf + pos, sizeof(buf) - pos, "\n");

    emerg_puts(buf);
}

void virtio_emerg_log_ext(const char *source, int pid, const char *level, const char *label, const char *fmt, ...) {
    char buf[512];
    int pos = 0;

    // [USER:PID=2] [INFO] shell: ...
    pos += ksnprintf(buf + pos, sizeof(buf) - pos, "[%s:PID=%d] [%s] %s: ", source, pid, level, label ? label : "");

    va_list args;
    va_start(args, fmt);
    pos += kvnprintf(buf + pos, sizeof(buf) - pos, fmt, args);
    va_end(args);

    pos += ksnprintf(buf + pos, sizeof(buf) - pos, "\n");

    emerg_puts(buf);
}

void virtio_emerg_log_va(const char *level, const char *unused_color, const char *fmt, va_list args) {
    (void)unused_color;  // do not use color, prevents compiler warning

    char buf[512];
    int pos = 0;
    pos += ksnprintf(buf + pos, sizeof(buf) - pos, "[%s] ", level);
    pos += kvnprintf(buf + pos, sizeof(buf) - pos, fmt, args);
    pos += ksnprintf(buf + pos, sizeof(buf) - pos, "\n");

    emerg_puts(buf);
}

void virtio_emerg_test(volatile uint32_t *regs) {
    // base + 0x100 + offsetof(emerg_wr) == base + 0x108
    volatile uint32_t *emerg = (volatile uint32_t *)
        ((uintptr_t)regs + VIRTIO_MMIO_DEVICE_CONFIG + offsetof(struct virtio_console_config, emerg_wr));

    *emerg = 'H';
    *emerg = 'i';
    *emerg = '\n';
    uart_printf("[virtio_console_emerge_test] emergency‑write done\n");
}