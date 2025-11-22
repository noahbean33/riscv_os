#pragma once

#include <stdint.h>
#include "regs.h"
#include "assert.h"

// The trap_frame struct represents the program state saved in kernel_entry. 
struct trap_frame {
    regs_t regs;    // General-purpose registers
    uint64_t sp;    // Stack pointer
    uint64_t epc;   // Saved program counter (resume point)
} __attribute__((packed));

typedef struct trap_frame trap_frame_t;

// Compile‑time checks:
STATIC_ASSERT(sizeof(regs_t)   == 30 * 8,   "regs_t must be 240 bytes");
STATIC_ASSERT(offsetof(trap_frame_t, sp)  == 30 * 8,   "trap_frame.sp at offset 240");
STATIC_ASSERT(offsetof(trap_frame_t, epc) == 31 * 8,   "trap_frame.epc at offset 248");

static inline void sfence_vma() {
    __asm__ volatile("sfence.vma zero, zero");
}

// READ_CSR and WRITE_CSR macros are convenient macros for reading and writing CSR registers.
#define READ_CSR(reg) \
    ({ uint64_t __tmp; __asm__ __volatile__("csrr %0, " #reg : "=r"(__tmp)); __tmp; })

#define WRITE_CSR(reg, value) \
    do { uint64_t __tmp = (value); __asm__ __volatile__("csrw " #reg ", %0" :: "rK"(__tmp)); } while (0)

void trap_init();
void dump_trap_frame(trap_frame_t* tf);