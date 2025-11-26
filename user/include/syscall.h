#pragma once
#include "types.h"
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

// POSIX SYSCALL
// official POSIX/Linux (RISC-V) syscall numbers
// (bron: Linux kernel arch/riscv/include/uapi/asm/unistd.h):
#define SYS_READ            63 
#define SYS_WRITE           64
#define SYS_EXIT            93
#define SYS_YIELD           124
#define SYS_GETPID          172
#define SYS_SYSINFO         200
#define SYS_GET_TIME        201
#define SYS_BRK             214          // still missing
#define SYS_FORK            220
#define SYS_EXEC            221 
#define SYS_WAIT            260

#define SYS_PUTCHAR         400  // Native syscall, outside POSIX
#define SYS_SBRK            401
#define SYS_IPC_SEND        402
#define SYS_IPC_RECV        403
#define SYS_PS              404
#define SYS_LS              405
#define SYS_CLEAR           406
#define SYS_TARFS_EXISTS    407


#define SYS_DUMP_TF         500  // Debug syscalls, outside POSIX
#define SYS_LOG             501

extern char __user_heap_start[];

int64_t syscall(int64_t num, int64_t arg0, int64_t arg1, int64_t arg2);
void* sbrk(long increment);
ssize_t read(int fd, void *buf, size_t count);
int write(int fd, const void *buf, size_t len);
void cls();
void yield(void);
int fork(void);
int exec(const char *program_name, char** argv, int argc);
void exit(int status);
int wait(int *wstatus);
int tarfs_exists(const char* filename);
void ps();
void ls();

