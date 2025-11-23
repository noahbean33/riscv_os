#include "kernel.h"
#include "uart.h"
#include "trap.h"
#include "bss.h"
#include "stack.h"
#include "ram.h"
#include "heap.h"
#include "test.h"
#include "types.h"  
#include "page.h"
#include "tar-parser.h"

pagetable_t kernel_pagetable;

void kernel_main(void) {
    bss_init();
    uart_cls();
    uart_printf("Novix RISC-V 64 OS, (c) NovixManiac, Version 0.0.1\n\n");
    uart_puts("Booting ...\n");

    trap_init();
    stack_init();
    ram_init();
    heap_init();
    paging_init();

    size_t fs;
    void *idle_elf_file = tarfs_lookup("idle.elf", &fs, 1);
    if (!idle_elf_file) {
        PANIC("[kernel_main] idle.elf not found!");
    }

    for(;;);
}