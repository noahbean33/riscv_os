#include "page.h"
#include "kernel.h"
#include "string.h"
#include "trap.h"
#include "assert.h"
#include "heap.h"
#include "uart.h"
#include "riscv.h"
#include "virtio-mmio.h"
#include "debug.h"

extern pagetable_t kernel_pagetable;
extern struct process *current_proc;   // Currently running process

uint64_t g_free_pages = 0;

void paging_init(void) {
    //  alloc and identity map for the kernel itself
    paddr_t pt_page = alloc_pages(1);

    kernel_pagetable = (pagetable_t)pt_page;

    for (paddr_t pa = (paddr_t)__kernel_base; pa < (paddr_t)__heap_end; pa += PAGE_SIZE) {
        map_page(kernel_pagetable, pa, pa, PTE_V|PTE_R|PTE_W|PTE_X, 0);
    }

     // map MMIO voor virtio-emerg debugging
    map_mmio_range(kernel_pagetable, 
        VIRTIO_MMIO_BASE, 
        VIRTIO_MMIO_BASE, 
        VIRTIO_MMIO_SIZE * VIRTIO_MMIO_MAX_DEVICES,
         PTE_V | PTE_R | PTE_W);

    // switch immediately to this page table
    set_active_pagetable((uintptr_t)kernel_pagetable);

    uart_printf("[paging_init] kernel page table initialized at : 0x%lx\n",
                (uintptr_t)kernel_pagetable);
}

// Get one page from the free list, or return 0 if empty
static paddr_t alloc_free_list(void) {
    if (free_list == 0)
        return 0;
    paddr_t pa = free_list;
    free_list = *(paddr_t *)pa;
    g_free_pages--;  // ✅ to keep up with
    return pa;
}
 
// Allocation of n contiguous pages
paddr_t alloc_pages(uint64_t n) {
    paddr_t pa;

    if (n == 1) {
        // Try a page from the free_list first
        pa = alloc_free_list();
        if (pa != 0) {
            memset((void*)pa, 0, PAGE_SIZE);

          //  uart_printf("[ALLOC] page at 0x%x (%d page(s))\n", (uint64_t)pa, n);
            return pa;
        }
        // Otherwise bump pointer of the heap
        pa = next_paddr;
        next_paddr += PAGE_SIZE;
        if (next_paddr > (paddr_t)__free_ram_end)
            PANIC("out of memory");
        memset((void*)pa, 0, PAGE_SIZE);

       // uart_printf("[ALLOC] page at 0x%x (%d page(s))\n", (uint64_t)pa, n);
        return pa;
    } else {
        // Multiple pages via bump pointer
        pa = next_paddr;
        paddr_t new_next = next_paddr + n * PAGE_SIZE;
        if (new_next > (paddr_t)__free_ram_end){
            PANIC("out of memory");
        }
        memset((void*)pa, 0, n * PAGE_SIZE);
        next_paddr = new_next;
       
       // uart_printf("[ALLOC] page at 0x%x (%d page(s))\n", (uint64_t)pa, n);
        return pa;
    }
}

// Add a physical page to the free list
void free_page(paddr_t pa) {
    // uart_printf("[FREE ] page at 0x%x\n", pa);
    *(paddr_t *)pa = free_list;
    free_list = pa;
    g_free_pages++;  // ✅ bijhouden
}

void free_pages_range(paddr_t pa, size_t npages) {
    for (size_t i = 0; i < npages; i++) {
        free_page(pa + i * PAGE_SIZE);
    }
}

void map_page(pagetable_t root_table, uint64_t va, uint64_t pa, uint64_t flags, int debug_flag) {
    if (va % PAGE_SIZE != 0) PANIC("unaligned va: 0x%lx", va);
    if (pa % PAGE_SIZE != 0) PANIC("unaligned pa: 0x%lx", pa);

    int vpn2 = (va >> 30) & 0x1FF;
    int vpn1 = (va >> 21) & 0x1FF;
    int vpn0 = (va >> 12) & 0x1FF;

    pagetable_t pt = root_table;

    // Level 2
    if (!(pt[vpn2] & PTE_V)) {
        uint64_t new_page = (uint64_t)alloc_pages(1);
        memset((void*)new_page, 0, PAGE_SIZE);
        pt[vpn2] = ((new_page >> 12) << 10) | PTE_V;
    }
    pt = (pagetable_t)((pt[vpn2] >> 10) << 12);

    // Level 1
    if (!(pt[vpn1] & PTE_V)) {
        uint64_t new_page = (uint64_t)alloc_pages(1);
        memset((void*)new_page, 0, PAGE_SIZE);
        pt[vpn1] = ((new_page >> 12) << 10) | PTE_V;
    }
    pt = (pagetable_t)((pt[vpn1] >> 10) << 12);

    // Level 0
    pt[vpn0] = ((pa >> 12) << 10) | flags | PTE_V;

    if (debug_flag) uart_printf("[map_page] VA=0x%x -> PA=0x%x flags=0x%x\n", va, pa, flags);
}

void unmap_page(pagetable_t root_table, vaddr_t va) {
    if (va % PAGE_SIZE != 0)
        PANIC("unaligned va: 0x%lx", va);

    int vpn2 = (va >> 30) & 0x1FF;
    int vpn1 = (va >> 21) & 0x1FF;
    int vpn0 = (va >> 12) & 0x1FF;

    pagetable_t pt = root_table;

    if (!(pt[vpn2] & PTE_V)) return;
    pt = (pagetable_t)((pt[vpn2] >> 10) << 12);

    if (!(pt[vpn1] & PTE_V)) return;
    pt = (pagetable_t)((pt[vpn1] >> 10) << 12);

    pt[vpn0] = 0; // verwijder PTE

    sfence_vma(); // update TLB
}

void set_active_pagetable(uintptr_t new_pagetable) {
    sfence_vma();
    WRITE_CSR(satp, MAKE_SATP(new_pagetable));
    sfence_vma();
}

void map_mmio_range(pagetable_t pt, uint64_t pa_start, uint64_t va_start, uint64_t len, uint64_t perm) {
    for (uint64_t offset = 0; offset < len; offset += PAGE_SIZE) {
        map_page(pt,
                 va_start + offset,
                 pa_start + offset,
                 perm,
                 0);
    }
}

void debug_pagetable(const char *who) {
    // read satp and extract the PPN (lower 44 bits are PPN)
    uint64_t satp = READ_CSR(satp);
    uint64_t ppn  = satp & ((1ULL << 44) - 1);

    // shift back to physical base address
    uint64_t root_pa = ppn << 12;

    LOG_USER_DBG("[%s] active page table @ 0x%x", who, root_pa);
}

// Recursively free a page-table. `lvl` is index level: 2=L2 (root), 1=L1, 0=L0 (leaf).
void free_pagetable(pagetable_t pt, int lvl) {
    for (int i = 0; i < 512; i++) {
        pte_t e = pt[i];
        if (!(e & PTE_V)) continue;   // not valid, nothing to do

        paddr_t child_pa = (paddr_t)((e >> PTE_PPN_SHIFT) << PAGE_SHIFT);

        if ((e & (PTE_R | PTE_W | PTE_X)) && lvl == 0) {
            // leaf entry (pointing to a page). Free only user-pages!
            if (e & PTE_U) {
                free_page(child_pa);
                // debug:
                // uart_printf("[FREE ] leaf page at 0x%x\n", child_pa);
            } else {
                // kernel-leaf: laat staan (niet vrijgeven)
            }
        } else {
            // sub-pagetable (pointer naar volgende level)
            pagetable_t next = (pagetable_t)PA2KA(child_pa);
            free_pagetable(next, lvl - 1);

            // now free the page table itself (this frame contains the next-level page table)
            free_page(child_pa);
            // debug:
            // uart_printf("[FREE ] pagetable at 0x%x (lvl=%d)\n", child_pa, lvl-1);
        }

        // clear PTE
        pt[i] = 0;
    }
}

paddr_t walk_page(pagetable_t pagetable, vaddr_t va) {
    for (int level = 2; level > 0; level--) {
        pte_t *pte = &pagetable[PX(level, va)];
        if (!(*pte & PTE_V)) return 0;
        pagetable = (pte_t *)PA2KA(PTE2PA(*pte));
    }
    pte_t pte = pagetable[PX(0, va)];
    if (!(pte & PTE_V)) return 0;
    return PTE2PA(pte);
}

pte_t* walk(pagetable_t pagetable, uint64_t va, int alloc) {
    if (va >= MAXVA)
        PANIC("walk: virtual address too high");

    for (int level = 2; level > 0; level--) {
        int idx = PX(level, va);
        pte_t *pte = &pagetable[idx];
        if (*pte & PTE_V) {
            pagetable = (pagetable_t)PA2KA(PTE2PA(*pte));
        } else {
            if (!alloc)
                return 0;
            // allocating a new page table
            paddr_t new = alloc_pages(1);
            memset(PA2KA(new), 0, PAGE_SIZE);
            *pte = ((new >> 12) << 10) | PTE_V;
            pagetable = (pagetable_t)PA2KA(new);
        }
    }

    return &pagetable[PX(0, va)];
}

// Find the physical address associated with a virtual user address
paddr_t walkaddr(pagetable_t pagetable, vaddr_t va) {
    if (va >= (vaddr_t)__kernel_base) {
        uart_printf("[walkaddr] reject VA=0x%lx: in kernel space\n", va);
        return 0; // Only user addresses allowed
    }

    pte_t *pte = walk(pagetable, va, 0);
    if (pte == 0) {
        uart_printf("[walkaddr] VA=0x%lx: walk() returned NULL\n", va);
        return 0;
    }

    if (!(*pte & PTE_V)) {
        uart_printf("[walkaddr] VA=0x%lx: PTE not valid (pte=0x%lx)\n", va, *pte);
        return 0;
    }

    if (!(*pte & (PTE_R | PTE_W | PTE_X))) {
        uart_printf("[walkaddr] VA=0x%lx: no RWX permissions (pte=0x%lx)\n", va, *pte);
        return 0;
    }

    paddr_t pa = PTE2PA(*pte);
    paddr_t final = pa + (va & 0xFFF);

   // uart_printf("[walkaddr] VA=0x%lx -> PA=0x%lx (pte=0x%lx)\n", va, final, *pte);
    return final;
}