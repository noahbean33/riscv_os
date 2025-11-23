#include <stddef.h>
#include "uart.h"

extern char __stack_bottom[], __stack_top[];    // kernel.ld

void stack_init(void){

    size_t stack_size = __stack_top - __stack_bottom ;

    uart_printf("[stack_init] STACK initialized, size = %d KB, start = 0x%x, end = 0x%x\n",
            stack_size / 1024,
            (uint32_t)(uintptr_t)__stack_bottom,
            (uint32_t)(uintptr_t)__stack_top);
}