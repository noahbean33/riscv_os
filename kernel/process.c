#include "process.h"
#include "kernel.h"
#include "memlayout.h"
#include "string.h"
#include "page.h"
#include "trap.h"
#include "heap.h"
#include "user.h"
#include "debug.h"
#include "alloc-tracker.h"

extern struct process *current_proc;    // Currently running process
extern int process_count;               // Number of processes
struct process procs[PROCS_MAX];        // All process control structures.
extern uint64_t g_free_pages;

proc_t *get_proc_by_pid(int pid) {
    if (pid <= 0 || pid > PROCS_MAX) return NULL;
    return &procs[pid - 1];
}

proc_t *alloc_free_proc(void) {
    for (int i = 0; i < PROCS_MAX; i++) {
        if (procs[i].state == PROC_UNUSED) {
            proc_t *p = &procs[i];
            memset(p, 0, sizeof(proc_t));  // reset all fields
            p->pid = i + 1;                // simple PID: index + 1
            return p;
        }
    }
    return NULL;
}

void strip_elf_extension(const char *progname, char *out_name, size_t maxlen) {
   // uart_printf("strip_elf_extension: progname= %s\n", progname);
    if (!progname || !out_name || maxlen == 0) return;

    strncpy(out_name, progname, maxlen - 1);
    out_name[maxlen - 1] = '\0';        //  null-terminated

    char *dot = strstr(out_name, ".elf");
    if (dot && strcmp(dot, ".elf") == 0) {
        *dot = '\0';                    // truncate at ".elf"
    }
   // uart_printf("strip_elf_extension: outname= %s\n", out_name);
}

struct process *create_init_process(const void *flat_image, size_t image_size, int debug_flag) {
    // 1) Find a free PCB
    struct process *p = NULL;
    for (int i = 0; i < PROCS_MAX; i++)
        if (procs[i].state == PROC_UNUSED) { 
            p = &procs[i]; 
            break; 
        }
    if (!p) PANIC("[create_init_process] No free PCB");

    // 1a) Keep track of number of process
    process_count += 1;

    // 2) Kernel stack prepare for 'trapframe'
    uint64_t *tf_ksp = (uint64_t*)&p->tf_stack[sizeof p->tf_stack];

    // Reserve space for trap_frame on the stack (for example, at the bottom)
    // trap_frame_t is a struct of (for example) 31 * 8 bytes (adapt to your trap_frame definition)
    tf_ksp -= sizeof(trap_frame_t) / sizeof(uint64_t);
    trap_frame_t *tf = (trap_frame_t *)tf_ksp;
    p->tf = tf;

    // 2a) Init trapframe
    memset(p->tf, 0, sizeof(trap_frame_t));
    p->tf->sp  = g_user_stack_top;              // user stack pointer
    p->tf->regs.ra  = (uint64_t)user_entry;     // ra = user_entry
    p->tf->epc = (uint64_t)USER_BASE;           // epc
    p->sp = (uint64_t)tf_ksp;                   // sp
   
    if (current_proc != NULL) 
    {
        p->parent = current_proc;
        p->parent_pid = current_proc->pid;
    }

    if (debug_flag) {
        LOG_DEBUG("[create_init_process] p->tf is at %p", p->tf);
        LOG_DEBUG("[create_init_process] p->tf->epc = %p", (void*)p->tf->epc);
        LOG_DEBUG("[create_init_process] p->tf->ra = %p", (void*)p->tf->regs.ra);
        LOG_DEBUG("[create_init_process] p->tf->sp is at %p", p->tf->sp);
    }

    // 3) Build your *own* user PT
    pagetable_t pt = (pagetable_t)alloc_pages(1);
    p->page_table = pt;
    LOG_INFO("[create_init_process] created user pagetable at : 0x%x", pt);

    // 3a) Identity map does include the complete kernel area
    for (paddr_t pa = (paddr_t)__kernel_base;
         pa < (paddr_t)__free_ram_end;
         pa += PAGE_SIZE) {
        map_page(pt, pa, pa, PTE_V | PTE_R | PTE_W | PTE_X, 0);
    }

    // 3b) For each page-aligned segment in flat_image:
    LOG_INFO("[create_init_process] mapping elf started...");
    size_t num_pages = (image_size + PAGE_SIZE - 1) / PAGE_SIZE;
    for (size_t i = 0; i < num_pages; i++) {
        paddr_t page = alloc_pages(1);
        size_t c = (i == num_pages - 1)
            ? (image_size - i * PAGE_SIZE)
            : PAGE_SIZE;
        memcpy((void*)page, (uint8_t*)flat_image + i * PAGE_SIZE, c);
        map_page(pt,
                 USER_BASE + i * PAGE_SIZE,
                 page,
                 PTE_V | PTE_U | PTE_R | PTE_W | PTE_X,
                0);
    }
    LOG_INFO("[create_init_process] mapping elf done!");

    // 4) Map the user stack to exactly __user_stack_top - PAGE_SIZE
    uint64_t user_stack_size = g_user_stack_top - g_user_stack_bottom;

    if (debug_flag) {
        LOG_DEBUG("[create_init_process] stack_bottom aligned? %s", (g_user_stack_bottom % PAGE_SIZE == 0) ? "Yes" : "No");
        LOG_DEBUG("[create_init_process] stack_top    aligned? %s", (g_user_stack_top % PAGE_SIZE == 0) ? "Yes" : "No");
    }
    LOG_INFO("[create_init_process] mapping userstack top = 0x%x, bottom = 0x%x", 
        g_user_stack_top, g_user_stack_bottom);

    for (uint64_t va = g_user_stack_bottom; va <= g_user_stack_top; va += PAGE_SIZE) {
        paddr_t pa = alloc_pages(1);
        map_page(p->page_table, va, pa, PTE_V | PTE_R | PTE_W | PTE_U, 0);
    }
   
    LOG_INFO("[create_init_process] User stack: 0x%x - 0x%x (%u bytes)",
            g_user_stack_bottom, g_user_stack_top, user_stack_size);

    // 5) Map the first heap page to g_user_heap_start
    paddr_t heap_page = alloc_pages(1);
    map_page(pt, g_user_heap_start, heap_page, PTE_V | PTE_U | PTE_R | PTE_W, 0);
    memset((void*)heap_page, 0, PAGE_SIZE);
    LOG_INFO("[create_init_process] Heap : 0x%x", heap_page);

    // 6) Complete the PCB and make it runnable
    p->pid        = 1 + (p - procs);
    p->state      = PROC_RUNNABLE;
    p->heap_end   = g_user_heap_start + PAGE_SIZE;
    p->user_stack_top = g_user_stack_top;
    p->entry_point    = USER_BASE;

    p->image_pa = (paddr_t)flat_image;
    p->image_npages = num_pages;
    
    //dump_trap_frame(tf);
    LOG_INFO("[create_init_process] Process pointer created : 0x%p", p);
    return p;
}

struct process *exec_process(const void *flat_image, size_t image_size, int debug_flag) {
   
    struct process *p = current_proc;
    p->tf = (trap_frame_t *)(p->tf_stack + sizeof(p->tf_stack) - sizeof(trap_frame_t));
    if (debug_flag) LOG_DEBUG("[exec_process] current proc = %d", p->pid);

    // 2) Build your *own* user PT
    pagetable_t pt = (pagetable_t)alloc_pages(1);
    p->page_table = pt;
    if (debug_flag) LOG_DEBUG("[exec_process] created pagetable at : 0x%x", pt);

    // 2a) Identity map does include the complete kernel area
    for (paddr_t pa = (paddr_t)__kernel_base;
         pa < (paddr_t)__free_ram_end;
         pa += PAGE_SIZE) {
        map_page(pt, pa, pa, PTE_V | PTE_R | PTE_W | PTE_X, 0);
    }

    if (debug_flag) LOG_DEBUG("[exec_process] mapping elf started...");
    // 2b) For each page-aligned segment in flat_image:
    size_t num_pages = (image_size + PAGE_SIZE - 1) / PAGE_SIZE;
    for (size_t i = 0; i < num_pages; i++) {
        paddr_t page = alloc_pages(1);
        size_t c = (i == num_pages - 1)
            ? (image_size - i * PAGE_SIZE)
            : PAGE_SIZE;
        memcpy((void*)page, (uint8_t*)flat_image + i * PAGE_SIZE, c);
        map_page(pt,
                 USER_BASE + i * PAGE_SIZE,
                 page,
                 PTE_V | PTE_U | PTE_R | PTE_W | PTE_X,
                0);
    }
    if (debug_flag) LOG_DEBUG("[exec_process] mapping elf done!");

    // 3) Map the user stack to exactly __user_stack_top - PAGE_SIZE
    uint64_t user_stack_size = g_user_stack_top - g_user_stack_bottom;

    if (debug_flag) {
        LOG_DEBUG("[exec_process] stack_bottom aligned? %s", (g_user_stack_bottom % PAGE_SIZE == 0) ? "Yes" : "No");
        LOG_DEBUG("[exec_process] stack_top    aligned? %s", (g_user_stack_top % PAGE_SIZE == 0) ? "Yes" : "No");
    }

    for (uint64_t va = g_user_stack_bottom; va <= g_user_stack_top; va += PAGE_SIZE) {
        paddr_t pa = alloc_pages(1);
        map_page(p->page_table, va, pa, PTE_V | PTE_R | PTE_W | PTE_U, debug_flag);
    }

    if (debug_flag) LOG_DEBUG("[exec_process] User stack: 0x%x - 0x%x (%u bytes)",
            g_user_stack_bottom, g_user_stack_top, user_stack_size);

    // 4a) Map the first heap page to g_user_heap_start
    paddr_t heap_page = alloc_pages(1);
    map_page(pt, g_user_heap_start, heap_page, PTE_V | PTE_U | PTE_R | PTE_W, 0);
    memset((void*)heap_page, 0, PAGE_SIZE);
    if (debug_flag) LOG_DEBUG("[exec_process] Heap : 0x%x", heap_page);

    // 5) Complete the PCB and make it runnable
    p->state      = PROC_RUNNING;
    p->heap_end   = g_user_heap_start + PAGE_SIZE;
    p->user_stack_top = g_user_stack_top;
    p->entry_point    = USER_BASE;

    p->image_pa = (paddr_t)flat_image;
    p->image_npages = num_pages;
    
    //dump_trap_frame(tf);
    LOG_INFO("[exec_process] Process pointer created : 0x%p", p);
    return p;
}

void free_proc(proc_t *proc) {

    if (proc->tf) {
        proc->tf = NULL;
    }

    
    if (proc->image_pa) {
        free_pages_range(proc->image_pa, proc->image_npages);
        proc->image_pa = 0;
        proc->image_npages = 0;
    }

    if (proc->page_table) {
        process_free_userspace(proc);
        proc->page_table = NULL;
    }

    // tany outstanding allocations for this process
    free_all_tracked_for_pid(proc->pid, 0);
    dump_allocs_for_pid(proc->pid, 0);

    // Reset PCB fields (defensive)
    proc->state = PROC_UNUSED;
    proc->pid = 0;
    proc->parent = NULL;
    proc->name[0] = '\0';
    proc->heap_end = 0;
    proc->user_stack_top = 0;
    proc->argc = 0;
    proc->argv_ptr = 0;

    process_count -= 1;
    LOG_USER_DBG("[free_proc] g_free_pages %d", g_free_pages);
    LOG_USER_DBG("[free_proc] process_count = %d", process_count);
    LOG_USER_INFO("[free_proc] process cleaned up");
}

void process_free_userspace(proc_t *p) {
    pagetable_t root = p->page_table;
    if (!root) return;

    // 1) Unmap and free individual user leaf pages between USER_BASE .. max(heap,stack)
    vaddr_t start_va = USER_BASE;
    vaddr_t end_va   = p->heap_end > g_user_stack_top ? p->heap_end : g_user_stack_top;

    // Round up
    end_va = (end_va + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    for (vaddr_t va = start_va; va <= end_va; va += PAGE_SIZE) {
        // get PTE pointers
        int vpn2 = (va >> 30) & 0x1FF;
        int vpn1 = (va >> 21) & 0x1FF;
        int vpn0 = (va >> 12) & 0x1FF;

        pte_t l2e = root[vpn2];
        if (!(l2e & PTE_V)) continue;

        pagetable_t l1 = (pagetable_t)PA2KA((l2e >> PTE_PPN_SHIFT) << PAGE_SHIFT);
        pte_t l1e = l1[vpn1];
        if (!(l1e & PTE_V)) continue;

        pagetable_t l0 = (pagetable_t)PA2KA((l1e >> PTE_PPN_SHIFT) << PAGE_SHIFT);
        pte_t pte = l0[vpn0];
        if (!(pte & PTE_V)) continue;
        if (!(pte & PTE_U)) continue;  // only user-leaf

        // free leaf
        paddr_t pa = (paddr_t)((pte >> PTE_PPN_SHIFT) << PAGE_SHIFT);
        unmap_page(root, va);   // remove mapping
        free_page(pa);         // free physical page
    }

    // 2) Free pagetables (L0/L1/L2) themselves recursively.
    //    free_pagetable will only free user leaves; always free sub-pagetables.
    free_pagetable(root, /*lvl=*/2);

    // 3) Free the root pagetable frame itself.
    // root is kernel-virtual pointer; convert to physical when freeing.
    paddr_t root_pa = KA2PA((paddr_t)root);
    free_page(root_pa);

    // 4) clear pointer so we can't double-free later
    p->page_table = NULL;
}


void dump_pcb(proc_t *p) {
    LOG_USER_DBG("=== PCB Dump: PID %d ===", p->pid);
    LOG_USER_DBG("State           : %s", p->state == PROC_RUNNABLE ? "RUNNABLE" : "UNUSED");
    LOG_USER_DBG("SP (kernel)     : 0x%x", (uint64_t)p->sp);
    LOG_USER_DBG("Heap end        : 0x%x", (uint64_t)p->heap_end);
    LOG_USER_DBG("Page table      : 0x%x", (uint64_t)p->page_table);
    LOG_USER_DBG("Entry point     : 0x%x", p->entry_point);
    LOG_USER_DBG("User stack top  : 0x%x", p->user_stack_top);

    if (p->tf) {
        dump_trap_frame(p->tf);
    } else {
        LOG_USER_ERR("([dump_pcb] trap frame is NULL)");
    }

    LOG_USER_DBG("=========================");
}

void print_process_table(void) {
    LOG_USER_INFO("[process list]");
    for (int i = 0; i < PROCS_MAX; i++) {
        struct process *p = &procs[i];

        const char *state = "???";
        switch (p->state) {
            case PROC_UNUSED:    state = "UNUSED  ";  break;
            case PROC_RUNNABLE:  state = "RUNNABLE";  break;
            case PROC_RUNNING:   state = "RUNNING ";  break;
            case PROC_WAITING:   state = "WAITING ";  break;
            case PROC_ZOMBIE:    state = "ZOMBIE  ";  break;
        }

        if (p->state != PROC_UNUSED) {
            LOG_USER_INFO("PID=%d  State=%s  EPC=0x%x  SP=0x%x, name=%s",
                p->pid, state, p->tf ? p->tf->epc : 0, p->tf ? p->tf->sp : 0, p->name);
        }
    }
}

const char* proc_state_str(int state) {
    switch (state) {
        case PROC_UNUSED:   return "UNUSED  ";
        case PROC_RUNNABLE: return "RUNNABLE";
        case PROC_RUNNING:  return "RUNNING ";
        case PROC_WAITING:  return "WAITING ";
        case PROC_ZOMBIE:   return "ZOMBIE  ";
        default:            return "UNKNOWN ";
    }
}
