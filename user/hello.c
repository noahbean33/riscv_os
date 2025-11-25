#include "include/stdio.h"
#include "include/syscall.h"

void main() {

    char *msg =  "[hello] Hello from exec!\n";
    puts(msg);
    
    exit(0);
}