#include "kernel.h"
#include "trap.h"
#include "syscall.h"
#include "uart.h"
#include "scheduler.h"
#include "util.h"
#include "page.h"
#include "process.h"
#include "string.h"
#include "riscv.h"
#include "tar-parser.h"
#include "elf-loader.h"
#include "page.h"
#include "trap.h"
#include "memlayout.h"
#include "user.h"
#include "sbi.h"
#include "types.h"
#include "heap.h"
#include "ram.h"
#include "time.h"
#include "common.h"
#include "virtio-emerg.h"
#include "debug.h"
#include "sysinfo.h"
#include "arguments.h"


extern struct process *current_proc; // Currently running process
extern int process_count;
extern pagetable_t kernel_pagetable;
extern struct process procs[];

#define DEBUG_PT(msg)    debug_page_table(msg)

void enable_sum(void) {
    uint64_t s = READ_CSR(sstatus);
    WRITE_CSR(sstatus, s | SSTATUS_SUM);
}

void disable_sum(void) {
    uint64_t s = READ_CSR(sstatus);
    WRITE_CSR(sstatus, s & ~SSTATUS_SUM);
}

long sys_sbrk(long increment) {
    
    proc_t *p = current_proc;
    uintptr_t old_heap = p->heap_end;
    uintptr_t new_heap = old_heap + increment;

    //uart_printf("[sys_sbrk] user heap_end = 0x%x, inc = %d\n", p->heap_end, increment);

    if (increment > 0) {
        for (uintptr_t addr = PGROUNDUP(old_heap); addr < new_heap; addr += PAGE_SIZE) {
            paddr_t pa = alloc_pages(1);
            if (!pa) return -1;
            void* new_page = (void*)(uintptr_t)pa;
            map_page(p->page_table, addr, (uintptr_t)new_page, PTE_U | PTE_R | PTE_W | PTE_V, 0);
        }
    }

    p->heap_end = new_heap;
    return old_heap;
}


ssize_t sys_read(int fd, char *buf, size_t len) {
    enable_sum();
    if (fd != 0 || buf == NULL || len == 0) {
        return -1;
    }

    size_t i = 0;
    while (i < len) {
        long c = uart_getc();
        if (c == -1) continue; // busy-wait tot input
        buf[i++] = (char)c;
        if (c == '\n') break;
        yield();
    }
    disable_sum();
    return i;
}

int sys_write(uint64_t fd, uint64_t user_buf, uint64_t len) {
    if (fd != 1) return -1;

    // Enable SUM
   enable_sum();

    char *buf = (char *)user_buf;

    for (uint64_t i = 0; i < len; i++) {
        char c = buf[i];  // direct access
        uart_putc(c);
    }

    // Restore SUM state
    disable_sum();

    return len;
}

int sys_fork(int debug_flag) {
    proc_t *parent = current_proc;

    // 1. Find a free PCB
    proc_t *child = alloc_free_proc();
    if (!child) return -1;
    process_count += 1;
    if (debug_flag) LOG_USER_DBG("[sys_fork] child allocated at: 0x%x", child);

    // 2. Allocating trapframe
    child->tf = (trap_frame_t *)alloc_pages(1);
    if (!child->tf) return -1;
    if (debug_flag) LOG_USER_DBG("[sys_fork] trapframe allocated at: 0x%x", child->tf);

    // 3. Dump original trapframe from parent (for debug)
    LOG_USER_INFO("[fork] source trapframe << PARENT >>");
    dump_trap_frame(parent->tf);

    // 4. Copy trapframe to child
    memcpy(child->tf, parent->tf, sizeof(trap_frame_t));

    // 5. Set correct return values
    child->tf->regs.a0 = 0;                     // fork() return values for child
    parent->tf->regs.a0 = child->pid;           // fork() return values for parent

    // 6. Explicitly set PC/SP of child
    child->tf->sp = parent->tf->sp;
    child->tf->epc = parent->tf->epc + 4;       // next instruction for child, not fork again

    if (debug_flag) {
        LOG_USER_DBG("[fork] parent sp=0x%x, epc=0x%x", parent->tf->sp, parent->tf->epc);
        LOG_USER_DBG("[fork] child  sp=0x%x, epc=0x%x", child->tf->sp, child->tf->epc);
    }
    
    // 7. Page table for child process
    pagetable_t new_pt = (pagetable_t)alloc_pages(1);
    if (debug_flag) LOG_USER_DBG("[fork] user page table initialized at : 0x%x", new_pt);
    child->page_table = new_pt;

    // 8. Inherit kernel mappings
    for (paddr_t pa = (paddr_t)__kernel_base;
         pa < (paddr_t)__free_ram_end;
         pa += PAGE_SIZE) {
        map_page(new_pt, pa, pa, PTE_V | PTE_R | PTE_W | PTE_X, 0);
    }

    // 9. Copy user pages
    vaddr_t start_va = USER_BASE;
    vaddr_t end_va = parent->heap_end > parent->user_stack_top
                   ? parent->heap_end
                   : parent->user_stack_top;

    for (vaddr_t va = start_va; va < end_va; va += PAGE_SIZE) {
        paddr_t pa_parent = walk_page(parent->page_table, va);
        if (!pa_parent) continue;

        paddr_t pa_child = alloc_pages(1);
        memcpy((void*)pa_child, (void*)PA2KA(pa_parent), PAGE_SIZE);
        map_page(new_pt, va, pa_child, PTE_V | PTE_U | PTE_R | PTE_W | PTE_X, 0);
    }

    // 10. PCB fields
    child->state = PROC_RUNNABLE;
    child->heap_end = parent->heap_end;
    child->user_stack_top = parent->user_stack_top;
    child->entry_point = parent->entry_point;

    child->parent = current_proc;
    child->parent_pid = current_proc->pid;

    strncpy(child->name, parent->name, sizeof(child->name) - 1);
    child->name[sizeof(child->name) - 1] = '\0';

    // 11. Finished!
    LOG_USER_INFO("[fork] parent->tf->a0 = %d", parent->tf->regs.a0);
    LOG_USER_INFO("[fork] child->tf->a0 = %d", child->tf->regs.a0);
    LOG_USER_INFO("[fork] pid = %d proc_t @ %p", child->pid, child);
    LOG_USER_INFO("[fork] fork with pid %d ready.", child->pid);
    
    return child->pid;
}

int sys_exec(const char *progname, char** argv, int argc) {
    char progname_buf[PROC_NAME_MAX_LEN];
    size_t fs;

    // 1. Search in tarfs
    enable_sum();           // Enable SUM

    strncpy(progname_buf, progname, sizeof(progname_buf) - 1);
    progname_buf[sizeof(progname_buf) - 1] = '\0';                  // null-terminate for safety
    const void *elf_data = tarfs_lookup(progname, &fs, 0);
    if (!elf_data) {
        uart_printf("[sys_exec] %s not found!\n", progname);
        return -1;
    }

    // 2. Store arguments in fixed buffer
    char (*argv_buf)[MAX_ARG_LENGTH] = (char (*)[MAX_ARG_LENGTH])alloc_pages(1);
    if (!argv_buf) {
        uart_puts("FATAL: failed to allocate argv_buf\n");
        return -1;
    }

    if (argc > 1) {
        for (int i = 0; i < argc; ++i) {
            strncpy(argv_buf[i], argv[i], MAX_ARG_LENGTH - 1);
            argv_buf[i][MAX_ARG_LENGTH - 1] = '\0';
        }
    }

    disable_sum();          // Disable SUM

    // 3. Enable kernel page table
    set_active_pagetable((uintptr_t)kernel_pagetable);
    
    // 4. Clean up the current process: reset the stack, set heap/bss to zero, etc.
    proc_t *proc = current_proc;
    process_free_userspace(proc);  
   
    // 5. Load the new ELF program
    struct process *ep = extract_flat_binary_from_elf(elf_data, EXEC_PROCESS);

    // 6. Put the program name minus '.elf' in the PCB
    strip_elf_extension(progname_buf, ep->name, sizeof(ep->name));
    
    // 7. Activate the new pagetable and stack
    set_active_pagetable((uintptr_t)ep->page_table);
    WRITE_CSR(sscratch, (uint64_t)(ep->tf_stack + sizeof(ep->tf_stack)));

    // 8a. setup sp and arguments
    uint64_t sp = g_user_stack_top;
    char **argv_ptr = NULL;

    if (argc > 1) {
        LOG_USER_DBG("[sys_exec] << START ARGV ARGC >>");
        argv_ptr = setup_user_arguments(argv_buf, argc, &sp);
        // 👇 Debug line to inspect the user stack
        //debug_argv_userstack(argc, argv_ptr);
    }

    // ✅ 8b. Argument buffer is niet meer nodig
    free_page((paddr_t)argv_buf);

    LOG_USER_INFO("[sys_exec] Init trapframe start.");
    
    // 9a. Setup trapframe
    trap_frame_t *tf = ep->tf;
    memset(tf, 0, sizeof(*tf));
    tf->epc      = USER_BASE;               // epc
    tf->regs.ra  = (uint64_t)user_return;   // ra
    tf->sp = sp;                            // sp

     // 9b. Setup argc and argv
    proc->argc = argc;                      // argc
    proc->argv_ptr = (uint64_t)argv_ptr;    // argv
   
    // debug
    LOG_USER_DBG("[sys_exec] New EPC: 0x%x, SP: 0x%x", ep->tf->epc, ep->tf->sp);
    LOG_USER_DBG("[sys_exec] CSR stvec   = 0x%x", READ_CSR(stvec));
    LOG_USER_DBG("[sys_exec] CSR sstatus = 0x%x", READ_CSR(sstatus));
    LOG_USER_DBG("[sys_exec] argv_buf pa = 0x%x", (uint64_t)argv_buf);
    debug_pagetable("sys_exec");
    dump_pcb(current_proc);
    LOG_USER_INFO("[sys_exec] ready...");

    yield();

    return 0;
}

void sys_exit(int code) {
    proc_t *proc = current_proc;

    LOG_USER_INFO("[sys_exit] pid=%d exit(%d)", proc->pid, code);

    proc->state = PROC_ZOMBIE;
    proc->exit_code = code;
}

int sys_wait(int *status, int debug_flag) {
    if (debug_flag) LOG_USER_DBG("[sys_wait] started ...");

    proc_t *parent = current_proc;

    while (1) {
        for (int i = 0; i < PROCS_MAX; i++) {
            proc_t *p = &procs[i];

            // Validation: Skip empty entries
            if (p == NULL) continue;

            // Check if parent pointer appears valid
            if (p->state == PROC_ZOMBIE && p->parent == parent) {
                if (debug_flag) LOG_USER_DBG("[sys_wait] found zombie child pid=%d", p->pid);

                // where exit code is requested as status
                if (status != NULL) {
                    enable_sum();
                    *status = p->exit_code;
                    disable_sum();
                }

                int dead_pid = p->pid;

                // Cleanup process
                free_proc(p);

                if (debug_flag) LOG_USER_DBG("[sys_wait] cleaned up pid=%d", dead_pid);
                return dead_pid;
            }
        }

        // No zombies found: temporarily free up CPU
        yield();
    }
}

void sys_ps(void) {
    uart_puts("PID\tSTATE\t\tNAME\n");
    for (int i = 0; i < PROCS_MAX; i++) {
        proc_t *p = &procs[i];
        if (p->pid != 0) {
            uart_printf("%d\t%s\t%s\n", p->pid, proc_state_str(p->state), p->name);
        }
    }
}

void sys_ls(void) {
    uint8_t *ptr = _binary_initramfs_tar_start;

    uart_puts("Files in tarfs:\n");

    while (ptr < _binary_initramfs_tar_end) {
        struct tar_header *hdr = (struct tar_header *)ptr;

        if (hdr->name[0] == '\0') break;  // tar-end

        // Show file name
        uart_printf("  %s\n", hdr->name);

        // Skip file (round to 512-byte blocks)
        size_t size = 0;
        for (int i = 0; i < 11; ++i) {
            size = (size << 3) + (hdr->size[i] - '0');
        }
        size_t blocks = (size + TAR_BLOCK_SIZE - 1) / TAR_BLOCK_SIZE;
        ptr += (1 + blocks) * TAR_BLOCK_SIZE;
    }
}

uint64_t sys_sysinfo(uint64_t arg0, ...) {
    struct sysinfo info = {
        .total_pages = page_allocator_total_pages(),
        .free_pages = page_allocator_free_pages(),
    };

    // copy_to_user is your safe userspace copy
    return copy_to_user(current_proc->page_table, (void *)arg0, &info, sizeof(info));
}

int64_t sys_time(uint64_t arg0, ...) {
    struct DateTime dt;

    // Get current time in ticks
    uint64_t ticks = get_time();
    //uart_printf("[sys_time] ticks = %lu\n", ticks);    //debug
    compute_datetime(ticks, &dt);

    // copy_to_user is your safe userspace copy
    return copy_to_user(current_proc->page_table, (void *)arg0, &dt, sizeof(dt));
}

long sys_log(const char *msg) {
    if (!msg) return -1;

    enable_sum();

    char buf[256];
    strncpy(buf, msg, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    disable_sum();

    struct process *p = current_proc;
    const char *label = p->name;

    // Temporarily switch to kernel pagetable
    uintptr_t old_satp = switch_pagetable((uintptr_t)kernel_pagetable);

    virtio_emerg_log_ext("USER", p->pid, "INFO", label, "%s", buf);

    // Restore original (user) page table
    restore_pagetable(old_satp);

    return 0;
}


void handle_syscall(struct trap_frame *f) {

    switch (f->regs.a7) {       // syscall number :  POSIX / System V ABI conventions (RISC-V 64)

        case SYS_READ:
            f->regs.a0 =  sys_read((int)f->regs.a0, (char *)f->regs.a1, (size_t)f->regs.a2);
            break;

        case SYS_WRITE:
            f->regs.a0 = sys_write(f->regs.a0, f->regs.a1, f->regs.a2);
            break;

        case SYS_YIELD:
            yield();
            f->regs.a0 = 0;
            break;

         case SYS_FORK:
            f->regs.a0 = sys_fork(1);   
            break;

         case SYS_EXEC:
            enable_sum();

            const char* filename = (const char*)f->regs.a0;
            char** argv = (char**)f->regs.a1;
            int argc = f->regs.a2;

            f->regs.a0 = sys_exec(filename, argv, argc);

            disable_sum();
            break;

         case SYS_EXIT:
            sys_exit(f->regs.a0);
            f->regs.a0 = 0;
            break;

        case SYS_WAIT:
            f->regs.a0 = sys_wait((int *)f->regs.a0, 1);
            break;

        case SYS_CLEAR:
            uart_cls();
            f->regs.a0 = 0;
            break;

        case SYS_SYSINFO:
            f->regs.a0 = sys_sysinfo((uint64_t)f->regs.a0);
            break;

         case SYS_GET_TIME:
            f->regs.a0 = sys_time((uint64_t)f->regs.a0);
            break;

        case SYS_PUTCHAR: 
            uart_putc(f->regs.a0);
            f->regs.a0 = 0;
            break;

        case SYS_SBRK :
            f->regs.a0 = sys_sbrk(f->regs.a0);
            break;

        case SYS_PS:
            sys_ps();
            f->regs.a0 = 0;
            break;

        case SYS_LS:
            sys_ls();
            f->regs.a0 = 0;
            break;

        case SYS_TARFS_EXISTS: {
            const char* filename = (const char*)f->regs.a0;
            enable_sum();
            void* file = tarfs_lookup(filename, NULL, 0);
            disable_sum();
            f->regs.a0 = (file != NULL) ? 1 : 0;
            break;
        }

         case SYS_LOG: {
            const char *msg = (const char *)f->regs.a0;  // syscall-param
            long ret = sys_log(msg);                     // call
            f->regs.a0 = ret;                            // return in a0
            break;
        }

        default:
            PANIC("[syscall] unknown syscall %d\n", f->regs.a7);
    }
}