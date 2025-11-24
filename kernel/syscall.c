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

void handle_syscall(struct trap_frame *f) {

    switch (f->regs.a7) {       // syscall number :  POSIX / System V ABI conventions (RISC-V 64)

        case SYS_READ:
            f->regs.a0 =  sys_read((int)f->regs.a0, (char *)f->regs.a1, (size_t)f->regs.a2);
            break;

        case SYS_WRITE:
            f->regs.a0 = sys_write(f->regs.a0, f->regs.a1, f->regs.a2);
            break;

        case SYS_CLEAR:
            uart_cls();
            f->regs.a0 = 0;
            break;

         case SYS_PUTCHAR: 
            uart_putc(f->regs.a0);
            f->regs.a0 = 0;
            break;

        case SYS_SBRK :
            f->regs.a0 = sys_sbrk(f->regs.a0);
            break;

        default:
            PANIC("[syscall] unknown syscall %d\n", f->regs.a7);
    }
}