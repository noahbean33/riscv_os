#include "include/stdio.h"

void main() {

    char *msg =  "[hello] Hello from exec!\n";
    puts(msg);
    
    for (;;);  // hang indefinitely so we can see that exec succeeded, no exit available yet
}