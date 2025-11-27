#pragma once
#include <stdint.h>
#include <stddef.h>
#include "types.h"

#define MAX_TRACKED_ALLOCS 4096*2

typedef struct {
    int     used;       // 1 = used
    paddr_t pa;         // fysieke start page adress
    size_t  npages;     // num of pages
    pid_t   owner;      // current_proc->pid or 0 for kernel
    char *tag;          // short string tag
    
} alloc_track_t;

void track_alloc(paddr_t pa, size_t npages, pid_t owner, char *tag);
void track_free(paddr_t pa, size_t npages);
void dump_allocs(void);

void dump_allocs_for_pid(int pid, int debug_flag);
void free_all_tracked_for_pid(pid_t pid, int debug_flag);