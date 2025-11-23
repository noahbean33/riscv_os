#pragma once

#include <stdint.h>
#include <stddef.h>
#include "page.h"

#define VIRTQ_DESC_F_NEXT           1
#define VIRTQ_DESC_F_WRITE          2
#define VIRTQ_AVAIL_F_NO_INTERRUPT  1
#define VIRTQ_MAX_DESC              8

/* Virtqueue ring-structs */
struct virtq_desc {
    uint64_t addr;
    uint32_t len;
    uint16_t flags;
    uint16_t next;
} __attribute__((packed));

struct virtq_avail {
    uint16_t flags;
    uint16_t idx;
    uint16_t ring[VIRTQ_MAX_DESC];
} __attribute__((packed));

struct virtq_used_elem {
    uint32_t id;
    uint32_t len;
} __attribute__((packed));

struct virtq_used {
    uint16_t flags;
    uint16_t idx;
    struct virtq_used_elem ring[VIRTQ_MAX_DESC];
} __attribute__((packed));

typedef struct virtq {
    struct virtq_desc  *desc;
    struct virtq_avail *avail;
    struct virtq_used  *used __attribute__((aligned(PAGE_SIZE)));

    /* per-descriptor buffers */
    void   *addr_virt[VIRTQ_MAX_DESC];
    uint64_t addr_phys[VIRTQ_MAX_DESC];
    uint8_t  desc_free[VIRTQ_MAX_DESC];

    int queue_index;
    int num_desc;

    /* basis MMIO-pointer */
    volatile uint32_t *regs;
} virtq_t;
