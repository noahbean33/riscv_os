#include "../include/syscall.h"

int64_t syscall(int64_t sysno, int64_t arg0, int64_t arg1, int64_t arg2) {
    register int64_t a0 __asm__("a0") = arg0;
    register int64_t a1 __asm__("a1") = arg1;
    register int64_t a2 __asm__("a2") = arg2;
    register int64_t a7 __asm__("a7") = sysno;

    __asm__ __volatile__("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a7) : "memory");

    return a0;
}

ssize_t read(int fd, void *buf, size_t count) {
    return syscall(SYS_READ, (int64_t)fd, (int64_t)buf, (int64_t)count);
}

int write(int fd, const void *buf, size_t len) {
    return (int)syscall(SYS_WRITE, fd, (intptr_t)buf, (intptr_t)len);
}

void cls() {
    syscall(SYS_CLEAR, 0, 0, 0);
}

void yield(void) {
    syscall(SYS_YIELD, 0, 0, 0);
}

int fork(void) {
    return syscall(SYS_FORK, 0, 0, 0);
}

int exec(const char *program_name) {
    return syscall(SYS_EXEC, (intptr_t)program_name, 0, 0);
}

void exit(int status) {
    syscall(SYS_EXIT, status, 0, 0);
    while (1) {}    // safety net if kernel does not return
}

int wait(int *wstatus) {
    return syscall(SYS_WAIT, (uintptr_t)wstatus, 0, 0);
}