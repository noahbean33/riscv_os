#include "include/stdio.h"
#include "include/syscall.h"

void main(int argc, char** argv) {

    for (int i = 1; i < argc; i++) {
        puts(argv[i]);
        if (i < argc - 1) putc(' ');
    }
    putc('\n');

    exit(0);
}