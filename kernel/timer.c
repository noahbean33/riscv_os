#include "timer.h"
#include "sbi.h"
#include "riscv.h"

void timer_init() {
    uint64_t now = 0;
    __asm__ __volatile__("rdtime %0" : "=r"(now));

    sbi_set_timer(now + TIMER_INTERVAL);

    // Enable S-mode timer interrupts
    enable_supervisor_timer_interrupts();
}