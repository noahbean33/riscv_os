#pragma once
#include <stddef.h>
#include <stdint.h>
#include "types.h"

// Typen voor user loglevels
typedef enum {
    LOG_LVL_INFO,
    LOG_LVL_WARN,
    LOG_LVL_ERR,
    LOG_LVL_DBG
} user_log_level_t;

void user_entry(void);
void log_user(user_log_level_t level, const char *fmt, ...);