#!/bin/bash

qemu-system-riscv64 -machine virt -m 512M -nographic -s -S -bios default -kernel bin/kernel.elf