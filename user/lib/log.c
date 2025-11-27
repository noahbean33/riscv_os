#include "../include/log.h"
#include "../include/syscall.h"
#include "../include/string.h"
#include "../include/common.h"
#include "../include/stdio.h"

void ulog(const char *fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    kvnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    // Here is your log output, e.g. syscall or print
    syscall(SYS_LOG, (uint64_t)buf, 0, 0);
}