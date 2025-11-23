#pragma once

#include <stddef.h>
#include <stdint.h>

void ram_init(void);
uint64_t page_allocator_total_pages();