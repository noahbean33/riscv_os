#pragma once

#include <stdint.h>

// Constants (virtio spec 1.1 / 1.2)
#define VIRTIO_MMIO_BASE            0x10001000  // QEMU virt machine
#define VIRTIO_MMIO_SIZE            0x1000       // meestal 4KB per device
#define VIRTIO_MMIO_MAGIC           0x000       // +0x000: magic value
#define VIRTIO_MMIO_VENDOR_ID       0x00c       // +0x00c: vendor ID (0x554d4551 = 'QEMU')
#define VIRTIO_MMIO_VERSION         0x004       // +0x004: version (1 = legacy)

#define VIRTIO_MMIO_DEVICE_ID       0x008       // +0x008: device type
#define VIRTIO_MMIO_DEVICE_FEATURES 0x010
#define VIRTIO_MMIO_DRIVER_FEATURES 0x020

#define VIRTIO_MMIO_GUEST_PAGE_SIZE 0x028

#define VIRTIO_MMIO_STATUS          0x070       // +0x070: device status register

#define VIRTIO_MMIO_QUEUE_SEL       0x030
#define VIRTIO_MMIO_QUEUE_NUM       0x038
#define VIRTIO_MMIO_QUEUE_PFN       0x040
#define VIRTIO_MMIO_QUEUE_NUM_MAX   0x034
#define VIRTIO_MMIO_QUEUE_ALIGN     0x03c
#define VIRTIO_MMIO_QUEUE_NOTIFY    0x050
#define VIRTIO_MMIO_QUEUE_READY     0x044

#define VIRTIO_MMIO_MAX_DEVICES         8
#define VIRTIO_MMIO_STRIDE          0x1000