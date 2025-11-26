#include "include/stdio.h"
#include "include/syscall.h"
#include "include/sysinfo.h"

int main() {
    struct sysinfo info;
    long ret = syscall(SYS_SYSINFO, (uintptr_t)&info, 0, 0);
    if (ret != 0) {
        printf("sys_sysinfo failed\n");
        return 1;
    }

    printf("=== Memory Status ===\n");
    printf("Total pages: %lu\n", info.total_pages);
    printf("Free  pages: %lu\n", info.free_pages);
    printf("Used  pages: %lu\n", info.total_pages - info.free_pages);
    return 0;
}