#pragma once

#include <stdint.h>
#include "trap.h"
#include "types.h"

// used for alloc_pages en free_pages
#define MAX_FREE_PAGES 1024

// head of the free list; 0 means "empty"
static paddr_t free_list = 0;  
static int free_count = 0;

#define PAGE_SIZE 4096
#define PAGE_SHIFT 12
#define SATP_MODE_SV39  8ULL

#define MAKE_SATP(pagetable)  ((SATP_MODE_SV39 << 60) | (((uintptr_t)(pagetable) >> 12) & 0xFFFFFFFFFFF))

#define SATP_SV39 (SATP_MODE_SV39 << 60)     // enable paging
#define PTE_V     (1UL << 0)                 // "Valid" bit (entry is enabled)
#define PTE_R     (1UL << 1)                 // Readable
#define PTE_W     (1UL << 2)                 // Writable
#define PTE_X     (1UL << 3)                 // Executable
#define PTE_U     (1UL << 4)                 // User (accessible in user mode)
#define PTE_TABLE (1UL << 1)                 // Only for internal flags

// In RISC‑V SV39, the PPN is in bits [54:10] of the PTE.
// So if you get a PTE >> PTE_PPN_SHIFT, the PPN is in the low bits.
#define PTE_PPN_SHIFT 10

// kernel.ld variables
extern char __free_ram[], __free_ram_end[];
extern char __kernel_base[], __user_base[];
extern char __heap_start[], __heap_end[];

#define PX(level, va) (((va) >> (12 + 9 * (level))) & 0x1FF)
 
// Your physical memory starts at __free_ram (e.g., 0x8024b000) and
// is directly mapped from that physical address in the kernel VADDR.
// However, if you really want to convert a PA to Kernel VA, you can
// (depending on your linker script) define something like this:
#define PA2KA(pa) ((void *)((uintptr_t)(pa)))  // your setup: PA == KA
// — In your setup, PA==KA, so no offset is needed.
// If your physical memory starts somewhere else, replace it with
// #define PA2KA(pa) ((void *)((pa) + KERNEL_VIRT_OFFSET))

#define KA2PA(ka) ((paddr_t)((uintptr_t)(ka)))


// The PTE2PA macro (Page Table Entry to Physical Address)
// Retrieves the physical address from a PTE:
#define PTE2PA(pte) (((pte) >> 10) << 12)

// In a RISC-V SV39 system (which supports 39 bits of virtual address space),
// the maximum virtual user address space is:
#define MAXVA (1L << 39)


// 'next_paddr' for the bump‐pointer fallback
static paddr_t next_paddr = (paddr_t)__free_ram;

// Stores the current SATP
static inline uintptr_t switch_pagetable(uintptr_t new_pagetable) {
    uintptr_t old_satp = READ_CSR(satp);
    WRITE_CSR(satp, MAKE_SATP(new_pagetable));
    __asm__ __volatile__("sfence.vma");
    return old_satp;
}

// Restores the previous SATP
static inline void restore_pagetable(uintptr_t old_satp) {
    WRITE_CSR(satp, old_satp);
    __asm__ __volatile__("sfence.vma");
}

// Virtual → Physical
static inline uintptr_t virt_to_phys(void *vaddr) {
    return (uintptr_t)vaddr;
}

// Physical → Virtual
static inline void *phys_to_virt(uintptr_t paddr) {
    return (void *)paddr;
}

void paging_init(void);
paddr_t alloc_pages(uint64_t n);
void map_page(pagetable_t root_table, uint64_t va, uint64_t pa, uint64_t flags, int debug_flag);
void set_active_pagetable(uintptr_t new_pagetable);

void map_mmio_range(pagetable_t pt, uint64_t pa_start, uint64_t va_start, uint64_t len, uint64_t perm);

pte_t* walk(pagetable_t pagetable, uint64_t va, int alloc);
paddr_t walkaddr(pagetable_t pagetable, vaddr_t va);
paddr_t walk_page(pagetable_t pagetable, vaddr_t va);

void unmap_page(pagetable_t root_table, vaddr_t va);
void free_page(paddr_t pa);
void free_pages_range(paddr_t pa, size_t npages);
void free_pagetable(pagetable_t pt, int lvl);
void debug_pagetable(const char *who);
