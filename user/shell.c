#include "include/stdio.h"
#include "include/syscall.h"

void main() {

    cls();
    puts("[shell] Welcome to the shell...\n");
    
    int pid = fork();
    if (pid == 0) {
        // child
        exec("hello.elf");
    } else {
        printf("[shell] created child with pid %d\n", pid);

        yield(); // temporary until timer is active
    }

    for(;;);
}