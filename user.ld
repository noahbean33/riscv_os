ENTRY(start)

SECTIONS {
    . = 0x1000000;

    /* machine code */
    .text : {
        KEEP(*(.text.start));
        *(.text .text.*);
    }

    .rodata : ALIGN(4) {
        *(.rodata .rodata.*);
    }

    .data : ALIGN(4) {
        *(.data .data.*);
    }

    .bss : ALIGN(4) {
        *(.bss .bss.* .sbss .sbss.*);
        ASSERT(. < 0x1800000, "too large executable");
    }

    /* 👇 Stack then (1MB + 64KB) */
    . = 0x1010000;
    .stack : {
        __user_stack_bottom = .;
        . += 64 * 1024;
        __user_stack_top = .;
        PROVIDE(__user_stack_top = .);
    }

    /* 👇 Heap segment for sbrk() */
    . = ALIGN(4096);
    __user_heap_start = 0x03000000;
    . += 32 * 1024 * 1024; /* 32MB user heap */
    __user_heap_end = .;

    /* 👇 Leave .riscv.attributes at the end */
    /DISCARD/ : {
        *(.riscv.attributes)
    }
}