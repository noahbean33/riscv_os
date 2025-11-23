#include "uart.h"
#include "heap.h"
#include "time.h"
#include "debug.h"

void heap_test(void){
    // heap test
    uart_printf("Heap Test :\n");
    void *a = malloc(64);
    void *b = malloc(128);
    uart_printf("a = %p\n", a);
    uart_printf("b = %p\n", b);
    free(a);
    uart_printf("a = free\n");
    void *c = malloc(32);
    uart_printf("c = %p\n", c);
}

void getc_test(void){
    uart_printf("Kernel booted. Type a key: ");

    int c;
    while ((c = uart_getc()) == -1) {
    //Wait until a key is pressed
    }

    uart_putc(c); // echo key
    uart_printf("\nReady with uart_getc() test\n");
}

void date_time_test(void){
    uint64_t ticks = get_time();  
    struct DateTime dt;
    compute_datetime(ticks, &dt);
    print_datetime(&dt);
}

void log_test(void){

    LOG_INFO("init: PID=%d, status=%s, hex=0x%x", 5, "running", 0xdeadbeef);
    LOG_WARN("init: PID=%d, status=%s, hex=0x%x", 5, "running", 0xdeadbeef);
    LOG_ERR("init: PID=%d, status=%s, hex=0x%x", 5, "running", 0xdeadbeef);
    LOG_DEBUG("init: PID=%d, status=%s, hex=0x%x", 5, "running", 0xdeadbeef);

    LOG_DEBUG("kernel: gestart");
    LOG_DEBUG("number=%d, hex=0x%x", 123, 0xbeef);
}