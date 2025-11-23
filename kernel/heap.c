#include <stdint.h>
#include "string.h"
#include "uart.h"


extern char __heap_start[], __heap_end[];   // kernel.ld

typedef struct Block {
    size_t size;
    struct Block *next;
    int free;
} Block;

#define ALIGN8(x) (((x) + 7) & ~7)
#define BLOCK_SIZE sizeof(Block)

static Block *free_list = NULL;

void heap_init(void) {
    free_list = (Block *) __heap_start;
    free_list->size = __heap_end - __heap_start - BLOCK_SIZE;
    free_list->next = NULL;
    free_list->free = 1;
    uart_printf("[heap_init]  HEAP  initialized, size =  %d MB, start = 0x%x, end = 0x%x\n",
            (free_list->size + BLOCK_SIZE) / (1024 * 1024),
            (uint32_t)(uintptr_t)__heap_start,
            (uint32_t)(uintptr_t)__heap_end);

}

void *malloc(size_t size) {
    size = ALIGN8(size);
    Block *curr = free_list;

    while (curr) {
        if (curr->free && curr->size >= size) {
            if (curr->size >= size + BLOCK_SIZE + 8) {
                // Split block
                Block *new_block = (Block *)((char *)curr + BLOCK_SIZE + size);
                new_block->size = curr->size - size - BLOCK_SIZE;
                new_block->free = 1;
                new_block->next = curr->next;

                curr->size = size;
                curr->next = new_block;
            }

            curr->free = 0;
            void *user_ptr = (char *)curr + BLOCK_SIZE;
            uart_printf("[malloc] size=%u → %p\n", size, user_ptr);
            return user_ptr;
        }
        curr = curr->next;
    }

    return NULL; // no suitable block found
}

void free(void *ptr) {
    if (!ptr) return;

    Block *block = (Block *)((char *)ptr - BLOCK_SIZE);
    block->free = 1;

    uart_printf("[free] %p size=%u\n", ptr, block->size);

    // Optional: coalesce with next
    if (block->next && block->next->free) {
        block->size += BLOCK_SIZE + block->next->size;
        block->next = block->next->next;
    }
}