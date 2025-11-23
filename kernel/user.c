#include "user.h"
#include "riscv.h"
#include "page.h"
#include "virtio-emerg.h"
#include "memlayout.h"

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