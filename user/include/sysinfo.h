#pragma once

#include <stdint.h>

struct sysinfo {
    uint64_t total_pages;
    uint64_t free_pages;
};