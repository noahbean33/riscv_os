#!/bin/bash

qemu-system-riscv64 -machine virt -m 512M -nographic -s -S -bios sbi/opensbi-riscv64-generic-fw_dynamic.bin -kernel bin/kernel.elf