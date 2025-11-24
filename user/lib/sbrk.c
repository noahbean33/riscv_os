#include "../include/syscall.h"

static uintptr_t heap_end = 0;

void *sbrk(long increment) {
    if (heap_end == 0)
        heap_end = (uintptr_t)&__user_heap_start;

    uintptr_t prev = heap_end;
    uintptr_t new_end = heap_end + increment;

    int64_t res = syscall(SYS_SBRK, increment, 0, 0);

    if (res < 0)
        return (void *)-1;

    heap_end = new_end;
    return (void *)prev;
}