#include "process.h"
#include "context.h"
#include "regs.h"
#include "uart.h"
#include "user.h"
#include "riscv.h"
#include "debug.h"

extern struct process *current_proc;    // Currently running process
extern struct process *idle_proc;       // Idle process
extern struct process procs[];          // Process Array
extern int process_count;
int g_startup = 1;                      // startup flag

void yield(void) {
    LOG_USER_INFO("[yield] <<<<< YIELD >>>>>");

    struct process *prev = current_proc;
    struct process *next = NULL;

    // look for another RUNNABLE process than the current one
    // idle = 0, startindex = 1 skipping idle process
    print_process_table();
    if (current_proc->state == PROC_RUNNING) current_proc->state = PROC_RUNNABLE;
    
    for (int i = 1; i < PROCS_MAX; i++) {
        proc_t *p = &procs[i];
        
        // skip empty slots en idle process (0)
        if (p == NULL || p->pid == 0) continue;

        // only idle & shell
        if (p->state == PROC_RUNNABLE && process_count == 2){ 
            next = p;
            break;
        }

        // run the next process
        if (p->state == PROC_RUNNABLE && p != current_proc) {
            next = p;
            break;
        }
    }
    dump_trap_frame(current_proc->tf);

    // If there is no other process, choose idle
    if (next == NULL) {
        next = idle_proc;
    }

    // If we stick with the same process, no switch is needed
    if (next == current_proc)
        return;

    // Set the status of the previous process (if necessary)
    if (prev->state == PROC_RUNNING) {
        prev->state = PROC_RUNNABLE;
    }

    // Set the new process status
    if (next->state == PROC_RUNNABLE) {
        next->state = PROC_RUNNING;
    }

    // Switch to the new address space
    __asm__ __volatile__(
        "sfence.vma\n"
        "csrw satp, %[satp]\n"
        "sfence.vma\n"
        "csrw sscratch, %[sscratch]\n"
        :
        : [satp] "r" (SATP_SV39 | ((uint64_t) next->page_table / PAGE_SIZE)),
          [sscratch] "r" ((uint64_t) &next->tf_stack[sizeof(next->tf_stack)])
    );

    current_proc = next;

    LOG_USER_INFO("[scheduler] switching to pid=%d tf@0x%x sp=0x%x epc=0x%x",
                next->pid, (uint64_t)next->tf, next->tf->sp, next->tf->epc);

    if (g_startup) {
        g_startup = 0; // now false, startup done
        LOG_USER_INFO("[scheduler] <<< SWITCH CONTEXT SP >>>");
        dump_trap_frame(next->tf);
        switch_context_sp(&prev->sp, &next->sp);
    } else {
        LOG_USER_INFO("[scheduler] <<< USER RETURN >>>");
        dump_trap_frame(current_proc->tf);
        print_process_table();
        user_return();
    }
}