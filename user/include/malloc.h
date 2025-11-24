#pragma once

#include <stddef.h>

typedef struct block {
    size_t size;
    struct block *next;
} block_t;

#define BLOCK_SIZE sizeof(block_t)

static block_t *free_list = NULL;

void *malloc(size_t size);
void free(void *ptr);