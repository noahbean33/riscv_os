#include "include/stdio.h"
#include "include/syscall.h"
#include "include/string.h"
#include "include/malloc.h"

void main() {

    cls();
    puts("[shell] Welcome to the shell...\n");
    
    int pid = fork();
    if (pid == 0) {
        // child
        puts("[shell] hello from child\n");
    } else {
        printf("[shell] created child with pid %d\n", pid);

        yield(); // temporary until timer is active
    }

    for(;;);
}