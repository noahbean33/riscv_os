#!/bin/bash
set -xue
clear

SOCK=/tmp/hosttime.sock

qemu-system-riscv64 -machine virt \
      -m 512M \
      -nographic \
      -s -S \
      -bios sbi/opensbi-riscv64-generic-fw_dynamic.bin \
      -kernel bin/kernel.elf \
      -serial mon:stdio \
      -device virtio-gpu-device \
      -device virtio-serial-device,id=virtserial0 \
      -device virtconsole,chardev=con0,bus=virtserial0.0 \
      -chardev file,id=con0,path=virtio_console.log \
      -chardev socket,id=hosttime,path="$SOCK",server=on,wait=off \
      -device virtserialport,chardev=hosttime,name=hosttime,bus=virtserial0.0 \
      -drive id=drive0,file=fs/initramfs.tar,format=raw,if=none \
      -device virtio-net-device,bus=virtio-mmio-bus.0 \
      -device virtio-blk-device,drive=drive0,bus=virtio-mmio-bus.1 

