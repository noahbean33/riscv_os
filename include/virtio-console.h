#pragma once

#include <stdint.h>
#include <stddef.h>

// +0x100: DEVICE_CONFIG base (na legacy registers)
#define VIRTIO_MMIO_DEVICE_CONFIG       0x100
#define VIRTIO_CONSOLE_F_EMERG_WRITE    (1 << 2)
#define VIRTIO_CONSOLE_BASE             0x10007000

struct virtio_console_config {
    uint16_t cols;              // offset 0x100
    uint16_t rows;              // offset 0x102
    uint32_t max_nr_ports;      // offset 0x104
    uint32_t emerg_wr;          // offset 0x108  ← hier schrijven we
} __attribute__((packed));