#pragma once

#define VIRT_RESET_ADDR 0x100000    // virt machine reset address

#define SSTATUS_SUM (1UL << 18)     // SUM: allow S-mode to access U-pages
#define SSTATUS_MXR (1UL << 19)     // MXR: make eXecute allowed as Read
#define SSTATUS_SPIE (1 << 5)       // SPIE: Saves Previous Interrupt Enable