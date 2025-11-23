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

    // clear bss
    bss_init();

    // clear screen, welcome message
    uart_cls();
    uart_printf("Novix RISC-V 64 OS, (c) NovixManiac, Version 0.0.1\n\n");
    uart_puts("Booting ...\n");

    // initialize trap_vector
    trap_init();

    // initialize stack, ram & heap
    stack_init();
    ram_init();
    heap_init();

    // intialize paging
    paging_init();

     // testing date & time
    date_time_test();
    
    // load idle process
    size_t fs;
    void *idle_elf_file = tarfs_lookup("idle.elf", &fs, 1);
    if (!idle_elf_file) {
        PANIC("[kernel_main] idle.elf not found!");
    }

    for(;;);
}