#include "alloc-tracker.h"
#include "uart.h"
#include "riscv.h"
#include "page.h"

alloc_track_t alloc_table[MAX_TRACKED_ALLOCS];
size_t alloc_table_count = 0;  // number of active entries

void track_alloc(paddr_t pa, size_t npages, pid_t owner, char *tag) {
    for (int i=0;i<MAX_TRACKED_ALLOCS;i++) {
        if (!alloc_table[i].used) {
            alloc_table[i].used = 1;
            alloc_table[i].pa = pa;
            alloc_table[i].npages = npages;
            alloc_table[i].owner = owner;
            alloc_table[i].tag = tag;
            alloc_table_count++;
            return;
        }
    }
}

void track_free(paddr_t pa, size_t npages) {
    for (int i=0;i<MAX_TRACKED_ALLOCS;i++) {
        if (alloc_table[i].used && alloc_table[i].pa == pa && alloc_table[i].npages == npages) {
            alloc_table[i].used = 0;
            alloc_table_count--;
            return;
        }
    }
}

void dump_allocs(void) {
    uart_printf("=== OUTSTANDING ALLOCS ===\n");
    for (int i=0;i<MAX_TRACKED_ALLOCS;i++) {
        if (alloc_table[i].used) {
            uart_printf("PA=0x%x npages=%d owner=%d tag=%s\n",
                        alloc_table[i].pa,
                        (int)alloc_table[i].npages,
                        (int)alloc_table[i].owner,
                        alloc_table[i].tag ? alloc_table[i].tag : "?");
        }
    }
}

void dump_allocs_for_pid(int pid, int debug_flag) {
    if (debug_flag) uart_printf("=== ALLOCS for pid=%d ===\n", pid);
    for (int i = 0; i < MAX_TRACKED_ALLOCS; i++) {
        if (alloc_table[i].used && alloc_table[i].owner == pid) {
            if (debug_flag)
            uart_printf("  leak: pa=0x%x npages=%d tag=%s\n",
                        alloc_table[i].pa,
                        (int)alloc_table[i].npages,
                        alloc_table[i].tag ? alloc_table[i].tag : "?");
        }
    }
    if (debug_flag) uart_printf("=== END of list - ALLOCS for pid=%d ===\n", pid);
}


void free_all_tracked_for_pid(pid_t pid, int debug_flag) {
    disable_interrupts();
    if (debug_flag) {
        uart_printf("[free_proc] Allocations still owned by pid %d:\n", pid);
        uart_printf("[free_all_tracked] Force freeing leftover allocations for pid=%d\n", pid);
    }

    for (int i = 0; i < MAX_TRACKED_ALLOCS; i++) {
        alloc_track_t *e = &alloc_table[i];
        if (!e->used) continue;
        if (e->owner != pid) continue;
        if (debug_flag)
        uart_printf("  freeing leak: pa=0x%x npages=%d tag=%s",
                    e->pa, (int)e->npages, e->tag ? e->tag : "?");

        // Release per page in the alloc
        for (size_t p = 0; p < e->npages; p++) {
            paddr_t pa_page = e->pa + p * PAGE_SIZE;
            free_page(pa_page);
        }

        if (debug_flag) uart_puts(" = destroyed\n");

        // Clean up tracker entry
        e->used = 0;
        e->pa = 0;
        e->npages = 0;
        e->owner = 0;
        e->tag = NULL;
        alloc_table_count--;
    }

    enable_interrupts();
}