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
#include "virtio.h"
#include "debug.h"
#include "process.h"
#include "elf-loader.h"
#include "string.h"
#include "scheduler.h"
#include "util.h"

pagetable_t kernel_pagetable;

struct process *current_proc;   // Currently running process
struct process *idle_proc;      // Idle process
int process_count = 0;          // Number of created processes


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

    // initialzie virtio
    virtio_bus_init_scan();

    // load idle process
    size_t fs;
    void *idle_elf_file = tarfs_lookup("idle.elf", &fs, 1);
    if (!idle_elf_file) {
        PANIC("[kernel_main] idle.elf not found!");
    }

    // create idle process
    idle_proc = extract_flat_binary_from_elf(idle_elf_file, CREATE_PROCESS);
    idle_proc->pid = 0; // idle
    strcpy(idle_proc->name, "idle");
    current_proc = idle_proc;
    LOG_INFO("[kernel_main] idle.elf loaded.");

    // load shell process
    void *shell_file = tarfs_lookup("shell.elf", &fs, 1);
    if (!shell_file) {
        PANIC("[kernel_main] shell.elf not found!\n");
    }

    // create shell process
    struct process *s = extract_flat_binary_from_elf(shell_file, CREATE_PROCESS);
    strcpy(s->name, "shell");
    LOG_INFO("[kernel_main] shell.elf loaded.");

    // start scheduler
    LOG_INFO("Start scheduler ...");
    while(1) {
        yield();
    }

    system_halt();      // Should never be reached, so just to be safe
}