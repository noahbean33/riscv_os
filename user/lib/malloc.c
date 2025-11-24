#include "../include/malloc.h"
#include "../include/syscall.h"

void *malloc(size_t size) {
    block_t *prev = NULL;
    block_t *curr = free_list;

    //printf("[malloc] requested: %d\n", size);
    //printf("[malloc] size=%d, BLOCK_SIZE=%d\n", size, sizeof(block_t));

    // Align grootte
    size = (size + 7) & ~7;

    while (curr) {
        if (curr->size >= size) {
            // Block found — cut?
            if (curr->size >= size + BLOCK_SIZE + 8) {
                // Splits blok
                block_t *new_block = (block_t*)((char*)curr + BLOCK_SIZE + size);
                new_block->size = curr->size - size - BLOCK_SIZE;
                new_block->next = curr->next;
                curr->size = size;
                if (prev)
                    prev->next = new_block;
                else
                    free_list = new_block;
            } else {
                // Use whole block
                if (prev)
                    prev->next = curr->next;
                else
                    free_list = curr->next;
            }
            return (char*)curr + BLOCK_SIZE;
        }
        prev = curr;
        curr = curr->next;
    }

    // No block found → request more heap via sbrk
    void *mem = sbrk(size + sizeof(block_t));

    if (mem == (void*)-1)
        return NULL;

    block_t *new_blk = (block_t*)mem;
    new_blk->size = size;
    return (char*)new_blk + BLOCK_SIZE;
}

void free(void *ptr) {
    if (!ptr) return;

    block_t *blk = (block_t*)((char*)ptr - BLOCK_SIZE);
    blk->next = free_list;
    free_list = blk;
}