#include "trap.h"
#include "kernel.h"
#include "riscv.h"
#include "uart.h"
#include "debug.h"
#include "string.h"

extern struct process *current_proc;    // Currently running process

void trap_handler(trap_frame_t *f)
{
    uint64_t scause = READ_CSR(scause);
    uint64_t stval = READ_CSR(stval);

    if (f == NULL)
        PANIC("[trap_handler] trapframe = NULL\n");

    uart_printf("[trap_handler] SP = 0x%lx, EPC = 0x%lx\n", f->sp, f->epc);

    // Get the sp+epc saved on the stack by trap_vector ---
    memcpy(current_proc->tf, f, sizeof(trap_frame_t));

    //  dump en panic ---
    LOG_USER_INFO("[trap_handler] ");
    dump_trap_frame(f);
    print_process_table();

    uart_printf("[trap_handler] Unexpected trap: scause=%ld, stval=0x%x, sepc=0x%x\n",
                scause, stval, f->epc);

    switch (scause)
    {
    case 0:
        PANIC("[trap_handler] Instruction address misaligned\n");
        break;
    case 1:
        PANIC("[trap_handler] Instruction access fault\n");
        break;
    case 2:
        PANIC("[trap_handler] Illegal instruction\n");
        break;
    case 3:
        PANIC("[trap_handler] Breakpoint\n");
        break;
    case 5:
        PANIC("[trap_handler] Load access fault\n");
        break;
    case 7:
        PANIC("[trap_handler] Store/AMO access fault\n");
        break;
    case 8:
        PANIC("[trap_handler] Environment call from U-mode\n");
        break;
    case 12:
        PANIC("[trap_handler] Instruction page fault\n");
        break;
    case 13:
        PANIC("[trap_handler] Load page fault\n");
        break;
    case 15:
        PANIC("[trap_handler] Store/AMO page fault\n");
        break;
    default:
        PANIC("[trap_handler] scause=%ld, stval=0x%lx, sepc=0x%lx\n",
              scause, stval, f->epc);
    }
}

void dump_trap_frame(trap_frame_t *f)
{
    LOG_USER_DBG("=== TrapFrame Debug Dump ===");
    LOG_USER_DBG(" frame@%p  sp=%p  epc=%p",
                 f,
                 f->sp,
                 f->epc);
    LOG_USER_DBG(" ra=%p gp=%p tp=%p",
                 f->regs.ra,
                 f->regs.gp,
                 f->regs.tp);
    LOG_USER_DBG(" t0=%p t1=%p t2=%p t3=%p t4=%p t5=%p t6=%p",
                 f->regs.t0,
                 f->regs.t1,
                 f->regs.t2,
                 f->regs.t3,
                 f->regs.t4,
                 f->regs.t5,
                 f->regs.t6);
    LOG_USER_DBG(" a0=%p a1=%p a2=%p a3=%p a4=%p a5=%p a6=%p a7=%p",
                 f->regs.a0,
                 f->regs.a1,
                 f->regs.a2,
                 f->regs.a3,
                 f->regs.a4,
                 f->regs.a5,
                 f->regs.a6,
                 f->regs.a7);
    LOG_USER_DBG(" s0=%p s1=%p s2=%p s3=%p s4=%p s5=%p s6=%p s7=%p",
                 f->regs.s0,
                 f->regs.s1,
                 f->regs.s2,
                 f->regs.s3,
                 f->regs.s4,
                 f->regs.s5,
                 f->regs.s6,
                 f->regs.s7);
    LOG_USER_DBG(" s8=%p s9=%p s10=%p s11=%p",
                 f->regs.s8,
                 f->regs.s9,
                 f->regs.s10,
                 f->regs.s11);
    LOG_USER_DBG("============================");
}

__attribute__((naked))
__attribute__((aligned(4))) void
trap_vector(void)
{
    __asm__ __volatile__(
        "csrrw sp, sscratch, sp\n" // Swap sp with kernel backup in scratch
        "addi sp, sp, -8 * 32\n"   // Make room on the stack

        // Save registers
        "sd ra,  8 * 0(sp)\n"
        "sd gp,  8 * 1(sp)\n"
        "sd tp,  8 * 2(sp)\n"
        "sd t0,  8 * 3(sp)\n"
        "sd t1,  8 * 4(sp)\n"
        "sd t2,  8 * 5(sp)\n"
        "sd t3,  8 * 6(sp)\n"
        "sd t4,  8 * 7(sp)\n"
        "sd t5,  8 * 8(sp)\n"
        "sd t6,  8 * 9(sp)\n"
        "sd a0,  8 * 10(sp)\n"
        "sd a1,  8 * 11(sp)\n"
        "sd a2,  8 * 12(sp)\n"
        "sd a3,  8 * 13(sp)\n"
        "sd a4,  8 * 14(sp)\n"
        "sd a5,  8 * 15(sp)\n"
        "sd a6,  8 * 16(sp)\n"
        "sd a7,  8 * 17(sp)\n"
        "sd s0,  8 * 18(sp)\n"
        "sd s1,  8 * 19(sp)\n"
        "sd s2,  8 * 20(sp)\n"
        "sd s3,  8 * 21(sp)\n"
        "sd s4,  8 * 22(sp)\n"
        "sd s5,  8 * 23(sp)\n"
        "sd s6,  8 * 24(sp)\n"
        "sd s7,  8 * 25(sp)\n"
        "sd s8,  8 * 26(sp)\n"
        "sd s9,  8 * 27(sp)\n"
        "sd s10, 8 * 28(sp)\n"
        "sd s11, 8 * 29(sp)\n"

        "csrr a0, sscratch\n" // Load original userspace sp
        "sd a0, 8 * 30(sp)\n" // Save to trap_frame.sp

        "csrr a1, sepc\n"     // Load userspace program counter
        "sd a1, 8 * 31(sp)\n" // Save to trap_frame.epc

        "addi a0, sp, 8 * 32\n" // Calculate new kernel sp top
        "csrw sscratch, a0\n"   // Set kernel backup sp

        "mv a0, sp\n"         // Set trap_frame* as argument
        "call trap_handler\n" // Call the C-traphandler

        // Restore registers
        "ld ra,  8 * 0(sp)\n"
        "ld gp,  8 * 1(sp)\n"
        "ld tp,  8 * 2(sp)\n"
        "ld t0,  8 * 3(sp)\n"
        "ld t1,  8 * 4(sp)\n"
        "ld t2,  8 * 5(sp)\n"
        "ld t3,  8 * 6(sp)\n"
        "ld t4,  8 * 7(sp)\n"
        "ld t5,  8 * 8(sp)\n"
        "ld t6,  8 * 9(sp)\n"
        "ld a0,  8 * 10(sp)\n"
        "ld a1,  8 * 11(sp)\n"
        "ld a2,  8 * 12(sp)\n"
        "ld a3,  8 * 13(sp)\n"
        "ld a4,  8 * 14(sp)\n"
        "ld a5,  8 * 15(sp)\n"
        "ld a6,  8 * 16(sp)\n"
        "ld a7,  8 * 17(sp)\n"
        "ld s0,  8 * 18(sp)\n"
        "ld s1,  8 * 19(sp)\n"
        "ld s2,  8 * 20(sp)\n"
        "ld s3,  8 * 21(sp)\n"
        "ld s4,  8 * 22(sp)\n"
        "ld s5,  8 * 23(sp)\n"
        "ld s6,  8 * 24(sp)\n"
        "ld s7,  8 * 25(sp)\n"
        "ld s8,  8 * 26(sp)\n"
        "ld s9,  8 * 27(sp)\n"
        "ld s10, 8 * 28(sp)\n"
        "ld s11, 8 * 29(sp)\n"
        "ld sp,  8 * 30(sp)\n" // Restore userspace sp
        "fence.i\n"
        "sret\n" // Jump back to userspace @ epc
    );
}

void trap_init()
{
    WRITE_CSR(stvec, (uint64_t)trap_vector);
    uart_printf("[trap_init] Trap vector initialized at : 0x%x\n", (void *)trap_vector);
}