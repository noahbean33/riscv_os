#include "include/stdio.h"
#include "include/syscall.h"

void main() {

    printf("starting process A\n");
    while (1) {
        putc('B');
    }
}