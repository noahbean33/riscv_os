#!/bin/bash
riscv64-unknown-elf-gdb \
  -ex "target remote localhost:1234" \
  -ex "add-symbol-file bin/kernel.elf 0x80200000" \
  -ex "mem 0x00000000 0x80000000 rw" \
  -ex "mem 0x80000000 0x88000000 rw"