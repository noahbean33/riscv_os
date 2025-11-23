#pragma once
#include <stddef.h>
#include <stdint.h>

#include "process.h"
#include "types.h"

// Offset van 'tf' binnen struct process
#define OFFSET_TF       (offsetof(struct process, tf))

// Typen voor user loglevels
typedef enum {
    LOG_LVL_INFO,
    LOG_LVL_WARN,
    LOG_LVL_ERR,
    LOG_LVL_DBG
} user_log_level_t;

void user_entry(void);
void user_return(void);
int copy_to_user(pagetable_t pt, void *dst_user, void *src_kernel, size_t len);
void log_user(user_log_level_t level, const char *fmt, ...);