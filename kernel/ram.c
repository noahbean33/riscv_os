#include "ram.h"
#include "uart.h"
#include "page.h"

extern char __free_ram[], __free_ram_end[];     // kernel.ld

uint64_t g_total_pages = 0;

void ram_init(void) {

     size_t ram_size = __free_ram_end  - __free_ram;

     g_total_pages = ram_size / PAGE_SIZE;

     uart_printf("[ram_init]   RAM   initialized, size =  %d MB, start = 0x%x, end = 0x%x total pages = %d\n",
          ram_size / (1024 * 1024),
          (uint32_t)(uintptr_t)__free_ram,
          (uint32_t)(uintptr_t)__free_ram_end,
          g_total_pages);
}

uint64_t page_allocator_total_pages() {
    return g_total_pages; 
}