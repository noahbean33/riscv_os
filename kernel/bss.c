#include "string.h"

extern char __bss[], __bss_end[];   // kernel.ld

void bss_init(void){
    // clear bss
    memset(__bss, 0, (size_t) __bss_end - (size_t) __bss);
}