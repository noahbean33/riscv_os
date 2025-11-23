#include "virtio.h"
#include "virtio-mmio.h"
#include "virtio-console.h"
#include "virtio-emerg.h"
#include "uart.h"

volatile uint32_t *virtio_console_regs = NULL;

void virtio_bus_init_scan() {
    uart_puts("[virtio-scan] Scanning for virtio devices...\n");

    for (int i = 0; i < VIRTIO_MMIO_MAX_DEVICES; i++) {

        uint64_t base = VIRTIO_MMIO_BASE + i * VIRTIO_MMIO_STRIDE;
       
        uint32_t magic = *(volatile uint32_t *)(base + VIRTIO_MMIO_MAGIC);
        uint32_t version = *(volatile uint32_t *)(base + VIRTIO_MMIO_VERSION);
        uint32_t device_id = *(volatile uint32_t *)(base + VIRTIO_MMIO_DEVICE_ID);
        uint32_t vendor_id = *(volatile uint32_t *)(base + VIRTIO_MMIO_VENDOR_ID);

        if (magic != VIRTIO_MAGIC || version != 1) {
            continue; // not valid or no virtio device
        }

        if (device_id) {

            uart_printf("[virtio-scan] Device found at 0x%lx: ID=%d Vendor=0x%x\n",
                 base, device_id, vendor_id);

            switch (device_id) {
                case VIRTIO_DEV_ID_NET : 
                    uart_puts("  → Network device\n"); 
                    break;
                case VIRTIO_DEV_ID_BLOCK : 
                    uart_puts("  → Block device\n"); 
                    break;
                case VIRTIO_DEV_ID_CONSOLE : 
                    uart_puts("  → Console device\n"); 
                    virtio_console_regs = (volatile uint32_t *) VIRTIO_CONSOLE_BASE;
                    virtio_emerg_init(virtio_console_regs);
                    break;
                case VIRTIO_DEV_ID_RNG : 
                    uart_puts("  → RNG device\n"); 
                    break;
                case VIRTIO_DEV_ID_GPU : 
                    uart_puts("  → GPU device\n"); 
                    break;
                // Note : not supported yet in QEMU
                case VIRTIO_DEV_ID_RTC : 
                    uart_puts("  → RTC (Timer/Clock) device\n"); 
                    break;
                case VIRTIO_DEV_ID_INPUT : 
                    uart_puts("  → Input device\n"); 
                    break;
                default: uart_puts("  → Unknown or unsupported device\n"); break;
            }
        }
    }

    uart_puts("[virtio-debug] Scan completed.\n");
}