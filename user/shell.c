#include "include/stdio.h"
#include "include/syscall.h"
#include "include/string.h"
#include "include/malloc.h"

void main(void) {

    cls();
    puts("\nHello world from userland! (shell.elf)\n\n");

    // malloc/free test
    puts("malloc/free test start...\n\n");
    void *a = malloc(100);
    void *b = malloc(200);
    void *c = malloc(300);

    printf("a = %p\n", a);
    printf("b = %p\n", b);
    printf("c = %p\n", c);

    free(b);
    void *d = malloc(180);  // could reuse b
    printf("d = %p\n", d);

    puts("\nmalloc/free test done.\n\n");

    char* str = malloc(64);
    strcpy(str, "✅ malloc works correctly!\n");
    puts(str);
    
    for (;;);
}