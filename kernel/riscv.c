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

void enable_supervisor_timer_interrupts(void){
     uint64_t sie;
    __asm__ __volatile__("csrr %0, sie" : "=r"(sie));
    sie |= (1 << 5); // bit 5 = STIE (Supervisor Timer Interrupt Enable)
    __asm__ __volatile__("csrw sie, %0" :: "r"(sie));
}

void disable_supervisor_timer_interrupts(void) {
    uint64_t sie;
    __asm__ __volatile__("csrr %0, sie" : "=r"(sie));
    sie &= ~(1 << 5); // clear STIE bit (bit 5)
    __asm__ __volatile__("csrw sie, %0" :: "r"(sie));
}