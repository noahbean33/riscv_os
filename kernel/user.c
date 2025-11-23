#include "user.h"
#include "riscv.h"
#include "page.h"
#include "virtio-emerg.h"
#include "memlayout.h"
#include "debug.h"

extern pagetable_t kernel_pagetable;

// ↓ __attribute__((naked)) is very important!
__attribute__((naked)) void user_entry(void) {
    __asm__ __volatile__ (
        // 1) Set instruction pointer and privilege flags
        "csrw sepc, %[sepc]\n"
        "csrw sstatus, %[sstatus]\n"
        // 2) Sync instruction stream and jump to usermode
        "fence.i\n"
        "sret\n"
        :
        : [sepc]    "r"(USER_BASE),
          [sstatus] "r"(SSTATUS_SPIE)
    );
}

// ↓ __attribute__((naked)) is very important!
__attribute__((naked)) void user_return(void) {
    __asm__ __volatile__ (
        // Get stair frame from current_proc->tf
        "la   t0, current_proc\n"               // t0 = &current_proc
        "ld   t0, 0(t0)\n"                      // t0 = current_proc
        "ld   a0, %[tf_offset](t0)\n"           // a0 = current_proc->tf

        // Load all registers from trapframe via a0 = current_proc->tf
        "ld ra,   0*8(a0)\n"
        "ld gp,   1*8(a0)\n"
        "ld tp,   2*8(a0)\n"

        "ld t0,   3*8(a0)\n"
        "ld t1,   4*8(a0)\n"
        "ld t2,   5*8(a0)\n"
        "ld t3,   6*8(a0)\n"
        "ld t4,   7*8(a0)\n"
        "ld t5,   8*8(a0)\n"
        "ld t6,   9*8(a0)\n"

        // 🔥 Skip "ld a0, 10*8(a0)" — we do last
        "ld a1,  11*8(a0)\n"
        "ld a2,  12*8(a0)\n"
        "ld a3,  13*8(a0)\n"
        "ld a4,  14*8(a0)\n"
        "ld a5,  15*8(a0)\n"
        "ld a6,  16*8(a0)\n"
        "ld a7,  17*8(a0)\n"

        "ld s0,  18*8(a0)\n"
        "ld s1,  19*8(a0)\n"
        "ld s2,  20*8(a0)\n"
        "ld s3,  21*8(a0)\n"
        "ld s4,  22*8(a0)\n"
        "ld s5,  23*8(a0)\n"
        "ld s6,  24*8(a0)\n"
        "ld s7,  25*8(a0)\n"
        "ld s8,  26*8(a0)\n"
        "ld s9,  27*8(a0)\n"
        "ld s10, 28*8(a0)\n"
        "ld s11, 29*8(a0)\n"

        // Restore stack pointer and epc
        "ld sp,  240(a0)\n"
        "ld t0,  248(a0)\n"
        "csrw sepc, t0\n"

        // Set correct sstatus with SPIE
        "csrr t1, sstatus\n"
        "ori  t1, t1, 0x20\n"     // SPIE (bit 5)
        "andi t1, t1, ~0x100\n"   // clear SPP (bit 8)
        "csrw sstatus, t1\n"

        // return to userspace
        "fence.i\n"
        "sret\n"
        :
        : [tf_offset] "i"(OFFSET_TF)
    );
}

int copy_to_user(pagetable_t pt, void *dst_user, void *src_kernel, size_t len) {

   // uart_puts("[copy_to_user] << COPY >>");
    for (size_t i = 0; i < len; i++) {
        uint64_t va = (uint64_t)dst_user + i;
        paddr_t pa = walkaddr(pt, va);  // VA ==>> PA
        if (!pa) {
            uart_printf("[copy_to_user] walkaddr: unmapped user VA: 0x%lx", va);
            return -1;
        }

        *((uint8_t *)pa) = *((uint8_t *)src_kernel + i);
        LOG_USER_DBG("[copy_to_user] copy byte: %x -> VA: 0x%x (PA: 0x%x)", *((uint8_t *)src_kernel + i), va, pa);
    }
    return 0;
}

void log_user(user_log_level_t level, const char *fmt, ...) {
    uintptr_t old_satp = switch_pagetable((uintptr_t)kernel_pagetable);

    va_list args;
    va_start(args, fmt);

    switch (level) {
        case LOG_LVL_INFO:
            virtio_emerg_log_va("INFO", NULL, fmt, args);
            break;
        case LOG_LVL_WARN:
            virtio_emerg_log_va("WARN", NULL, fmt, args);
            break;
        case LOG_LVL_ERR:
            virtio_emerg_log_va("ERR", NULL, fmt, args);
            break;
        case LOG_LVL_DBG:
            virtio_emerg_log_va("DBG", NULL, fmt, args);
            break;
    }

    va_end(args);
    restore_pagetable(old_satp);
}