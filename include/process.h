#pragma once

#include <stdint.h>
#include <stddef.h>
#include "page.h"                       // typedef vaddr_t
#include "regs.h"                       // regs_t
#include "types.h"

#define PROCS_MAX 8                     // Maximum number of processes

// proc state enum
typedef enum {
    PROC_UNUSED,
    PROC_RUNNABLE,
    PROC_RUNNING,
    PROC_WAITING,
    PROC_ZOMBIE  
} proc_state_t;

#define PGROUNDUP(x) (((x) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define ROUND_UP(x, sz)   (((x) + (sz) - 1) & ~((sz) - 1))
#define ROUND_DOWN(x, align) ((x) & ~((align) - 1))

#define STACK_ALIGN 16

#define EXEC_PROCESS    0
#define CREATE_PROCESS  1

extern char __kernel_base[];            // Note : comes from kernel.ld
extern uintptr_t g_user_stack_top;      // Note : comes from user.ld
extern uintptr_t g_user_stack_bottom;   // Note : comes from user.ld
extern uintptr_t g_user_heap_start;     // Note : comes from user.ld
extern uintptr_t g_user_heap_end;       // Note : comes from user.ld

extern pagetable_t kernel_pagetable;    // Note : comes from kernel.c

#define MAX_MSG_SIZE 256
#define PROC_NAME_MAX_LEN 64  

typedef struct ipc_message {
    char data[MAX_MSG_SIZE];
    size_t size;                // size of the message
    pid_t sender;               // sender of the message
    int pending;                // 1 = true = message available
} ipc_message_t;

// Process Control Block (PCB)
typedef struct process {
    pid_t pid;                      // Process ID
    proc_state_t state;             // Process state: PROC_UNUSED, PROC_RUNNABLE, PROC_RUNNING, PROC_ZOMBIE
    pid_t parent_pid;               // PID van de ouder
    int exited;                     // 0 = actief, 1 = exited
    int exit_code;                  // Exit status
    struct process *parent;         // Pointer to the parrent process
    uintptr_t entry_point;          // Program entry point (ELF)
    uintptr_t user_stack_top;       // Top of user stack
    uintptr_t heap_end;             // Top of heap
    vaddr_t sp;                     // Stack pointer
    uint64_t *page_table;           // pointer to the first-level page table.
    trap_frame_t *tf;               // Pointer naar kernel trapframe stack
    uint8_t tf_stack[8192];         // Trapframe raw stack‑ruimte…
    paddr_t image_pa;               // Elf image pa
    size_t  image_npages;           // Elf num of pages
    ipc_message_t inbox;            // IPC messages
    char name[PROC_NAME_MAX_LEN];   // Process name
    int argc;                       // Argumenten count
    uint64_t argv_ptr;              // Pointer naar argv
}  proc_t;

struct process *create_init_process(const void *flat_image, size_t image_size, int debug_flag);
struct process *exec_process(const void *flat_image, size_t image_size, int debug_flag);
proc_t *get_proc_by_pid(int pid);
proc_t *alloc_free_proc(void);
void strip_elf_extension(const char *progname, char *out_name, size_t maxlen);

void dump_pcb(proc_t *p);
void print_process_table(void);
const char* proc_state_str(int state);

void process_free_userspace(proc_t *p);
void free_proc(proc_t *proc);