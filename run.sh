#!/bin/bash
set -xue
clear

QEMU=qemu-system-riscv64
CC=clang
CFLAGS="-std=c11 -O2 -g3 -Wall -Wextra --target=riscv64-unknown-elf \
    -fno-stack-protector -ffreestanding -nostdlib -mcmodel=medany -Iinclude -I../include"

mkdir -p bin
rm -f bin/kernel.elf bin/kernel.map

echo "=== Build kernel ==="
$CC $CFLAGS -Wl,-Tkernel.ld -Wl,-Map=bin/kernel.map -o bin/kernel.elf \
    boot/boot.c \
    kernel/kernel.c

echo "=== Start QEMU ==="
$QEMU -machine virt \
      -m 512M \
      -nographic \
      -bios default \
      -kernel bin/kernel.elf \
      --no-reboot