ENTRY(boot)

SECTIONS {
    . = 0x80200000;
    __kernel_base = .;
    
    .text :{
        KEEP(*(.text.boot));
        *(.text .text.*);
    }

    .rodata : ALIGN(4) {
        *(.rodata .rodata.*);
    }

    . = ALIGN(8);
    .initramfs : {
        _binary_initramfs_tar_start = .;
        KEEP(*(.initramfs_payload))  /* Note: Correct section name as provided by objcopy */
        _binary_initramfs_tar_end = .;
    }

    .data : ALIGN(4) {
        *(.data .data.*);
    }

    .bss : ALIGN(4) {
        __bss = .;
        *(.bss .bss.* .sbss .sbss.*);
        __bss_end = .;
    }

    /* Stack */
    . = ALIGN(4);
    __stack_bottom = .;
    . += 128 * 1024;
    __stack_top = .;

    /* Free RAM region */
    . = ALIGN(4096);
    __free_ram = .;
    . += 64 * 1024 * 1024;
    __free_ram_end = .;

    /* Heap */
    . = ALIGN(4096);
    __heap_start = .;
    . += 32 * 1024 * 1024;
    __heap_end = .;
}