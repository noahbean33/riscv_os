#include "kernel.h"

void system_halt(void) {
    PANIC("[kernel] SYSTEM CRASH !!\n");
}

void delay(void) {
    for (int i = 0; i < 30000000; i++)
        __asm__ __volatile__("nop"); // do nothing
}