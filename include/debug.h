#pragma once

#include "virtio-emerg.h"
#include "user.h"

void emerg_putc(char c);
void emerg_puts(const char *s);
void emerg_printf(const char *fmt, ...);

// used for kernel log after init_paging en SYS_LOG
#define LOG_INFO(...)   virtio_emerg_log("INFO", NULL, __VA_ARGS__)
#define LOG_WARN(...)   virtio_emerg_log("WARN", NULL, __VA_ARGS__)
#define LOG_ERR(...)    virtio_emerg_log("ERR", NULL, __VA_ARGS__)
#define LOG_DEBUG(...)  virtio_emerg_log("DBG", NULL, __VA_ARGS__)

// The user variant switch first back to kernel page table to avoid page faults
#define LOG_USER_INFO(...) log_user(LOG_LVL_INFO, __VA_ARGS__)
#define LOG_USER_WARN(...) log_user(LOG_LVL_WARN, __VA_ARGS__)
#define LOG_USER_ERR(...)  log_user(LOG_LVL_ERR, __VA_ARGS__)
#define LOG_USER_DBG(...)  log_user(LOG_LVL_DBG, __VA_ARGS__)