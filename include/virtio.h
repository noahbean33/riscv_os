#pragma once
#include <stdint.h>
#include "virtq.h"

// Init
#define VIRTIO_MMIO_BASE            0x10001000  // QEMU virt-machine console
#define VIRTIO_MAGIC                0x74726976
#define VIRTIO_VENDOR_ID            0x554d4551  // QEMU

// Status
#define VIRTIO_STATUS_ACK           1
#define VIRTIO_STATUS_DRIVER        2
#define VIRTIO_STATUS_DRIVER_OK     4
#define VIRTIO_STATUS_FEATURES_OK   8

// Device types
#define VIRTIO_DEV_ID_NET           1
#define VIRTIO_DEV_ID_BLOCK         2
#define VIRTIO_DEV_ID_CONSOLE       3
#define VIRTIO_DEV_ID_RNG           4
#define VIRTIO_DEV_ID_GPU           16
#define VIRTIO_DEV_ID_RTC           17
#define VIRTIO_DEV_ID_INPUT         18

// Register
#define VIRTIO_REG_QUEUE_SEL     0x030
#define VIRTIO_REG_QUEUE_NUM     0x034
#define VIRTIO_REG_QUEUE_ALIGN   0x03C
#define VIRTIO_REG_QUEUE_PFN     0x038
#define VIRTIO_REG_QUEUE_READY   0x044
#define VIRTIO_REG_QUEUE_NOTIFY  0x050

typedef struct virtio_device {
    uint32_t magic_value;     // 0x74726976 ("virt")
    uint32_t version;         // 1 for legacy
    uint32_t device_id;       // Device type (RTC = 0x203, Block = 2, Net = 1, Console = 3, etc.)
    uint32_t vendor_id;       // Vendor (usually 0x554d4551 = "QEMU")

    volatile uint32_t *mmio_base;      // MMIO base address (0x10001000, etc.)
    uint32_t device_features; // Bitmask from device
    uint32_t driver_features; // Bitmask from driver

    uint32_t queue_num_max;   // max # desc in queue
    uint32_t queue_notify_off;// offset for queue notify
    uint32_t interrupt_status;// interrupt flags
    int status;               // driver status register. 
    int irq;                  // IRQ line (virtio IRQ)

    // Meerdere queues
    virtq_t tx_queue;
    virtq_t rx_queue;
    virtq_t control_tx_queue;
    virtq_t control_rx_queue;

    const char *name;         // optioneel: "rtc", "blk", etc.
    void *config;             // pointer naar device-specifieke config struct

    int console_ready;        // console readt flag
} virtio_device_t;


// API
void virtio_bus_init_scan();
