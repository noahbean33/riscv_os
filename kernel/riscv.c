#include "riscv.h"

void disable_interrupts() {
    uint64_t sstatus;
    __asm__ __volatile__("csrr %0, sstatus" : "=r"(sstatus));
    sstatus &= ~(1 << 1); // clear SIE bit (bit 1)
    __asm__ __volatile__("csrw sstatus, %0" :: "r"(sstatus));
}

void enable_interrupts() {
    uint64_t sstatus;
    __asm__ __volatile__("csrr %0, sstatus" : "=r"(sstatus));
    sstatus |= (1 << 1); // set SIE bit (bit 1)
    __asm__ __volatile__("csrw sstatus, %0" :: "r"(sstatus));
}