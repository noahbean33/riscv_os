#include "include/stdio.h"
#include "include/syscall.h"

void main(void) {

    cls();
    puts("\nHello world from userland! (shell.elf)\n");

    puts("Input test.\n");
    puts("Enter your name : ");

    int c;
     while (1) {
        int n = read(0, &c, 1);     // read 1 byte
        if (n == 1) {
            putc(c);                // echo to stdout
        }
    }

    
    for(;;);
}