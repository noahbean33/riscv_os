#!/bin/bash
set -xue
clear

# === Variables ===
QEMU=qemu-system-riscv64
OBJCOPY=llvm-objcopy
AR=llvm-ar
CC=clang
CFLAGS="-std=c11 -O2 -g3 -Wall -Wextra --target=riscv64-unknown-elf \
    -fno-stack-protector -ffreestanding -nostdlib -mcmodel=medany \
    -Iinclude -I../include"
LDFLAGS="-Wl,-Tuser.ld"

# === Directories ===
INITRAMFS_DIR=fs
KERNEL_OUT_DIR=bin

# === Cleanup ===
rm -f user/lib/*.a
rm -f user/bin/*.o user/bin/crt0.o user/bin/init_crt0.o
rm -f user/*.elf user/*.map
rm -f "$INITRAMFS_DIR"/initramfs.tar "$INITRAMFS_DIR"/initramfs.o
rm -f "$KERNEL_OUT_DIR"/kernel.elf "$KERNEL_OUT_DIR"/kernel.map

# === Build userland ===
echo "Compile shared userlib sources ..."
for src in common.c log.c malloc.c printf.c sbrk.c string.c syscall.c; do
  $CC $CFLAGS -c user/lib/$src -o user/bin/${src%.c}.o
done

# === Compile startup crt0 ===
echo "Compile startup (crt0/init_crt0) ..."
$CC $CFLAGS -c user/lib/crt0.S -o user/bin/crt0.o
$CC $CFLAGS -c user/lib/init_crt0.S -o user/bin/init_crt0.o

# === Compile user programs ===
echo "Compile user programs ..."
$CC $CFLAGS -c user/idle.c -o user/bin/idle.o
$CC $CFLAGS -c user/shell.c -o user/bin/shell.o
$CC $CFLAGS -c user/hello.c -o user/bin/hello.o
$CC $CFLAGS -c user/ps.c -o user/bin/ps.o
$CC $CFLAGS -c user/ls.c -o user/bin/ls.o
$CC $CFLAGS -c user/mem.c -o user/bin/mem.o
$CC $CFLAGS -c user/date.c -o user/bin/date.o
$CC $CFLAGS -c user/echo.c -o user/bin/echo.o
$CC $CFLAGS -c user/testa.c -o user/bin/testa.o
$CC $CFLAGS -c user/testb.c -o user/bin/testb.o

# === Compile servers ===
$CC $CFLAGS -Iservers/init/include -c servers/init/init.c -o user/bin/init.o

# === Build static userlib ===
echo "Build static userlib ..."
$AR rcs user/lib/libuser.a user/bin/common.o user/bin/log.o user/bin/malloc.o user/bin/printf.o user/bin/sbrk.o user/bin/string.o user/bin/syscall.o

# === Link user programs ===
echo "Link user programs ..."
$CC $CFLAGS $LDFLAGS -Wl,-Map=user/idle.map -o user/idle.elf \
     user/bin/init_crt0.o user/bin/idle.o user/lib/libuser.a

$CC $CFLAGS $LDFLAGS -Wl,-Map=user/shell.map -o user/shell.elf \
     user/bin/init_crt0.o user/bin/shell.o user/lib/libuser.a

$CC $CFLAGS $LDFLAGS -Wl,-Map=user/init.map -o user/init.elf \
  user/bin/init_crt0.o user/bin/init.o user/lib/libuser.a

for prog in hello ps ls mem date echo testa testb; do
  $CC $CFLAGS $LDFLAGS -Wl,-Map=user/${prog}.map -o user/${prog}.elf \
    user/bin/crt0.o user/bin/${prog}.o user/lib/libuser.a
done

# === Create tarball for initramfs ===
echo "=== Create tarball for initramfs ==="
(cd user && tar cf "../$INITRAMFS_DIR/initramfs.tar" \
  idle.elf shell.elf hello.elf ps.elf ls.elf mem.elf date.elf echo.elf init.elf testa.elf testb.elf)

$OBJCOPY -I binary -O elf64-littleriscv --rename-section .data=.initramfs_payload \
  "$INITRAMFS_DIR/initramfs.tar" "$INITRAMFS_DIR/initramfs.o"

./scripts/gen_buildtime_header.sh

# === Build kernel ===
echo "=== Build kernel ==="
$CC $CFLAGS -Wl,-Tkernel.ld -Wl,-Map=bin/kernel.map -o bin/kernel.elf \
    boot/boot.c \
    kernel/kernel.c \
    kernel/string.c \
    kernel/common.c \
    kernel/sbi.c \
    kernel/uart.c \
    kernel/test.c \
    kernel/trap.c \
    kernel/bss.c \
    kernel/stack.c \
    kernel/ram.c \
    kernel/heap.c \
    kernel/page.c \
    kernel/tar-parser.c \
    kernel/time.c \
    kernel/virtio.c \
    kernel/virtio-emerg.c \
    kernel/user.c \
    kernel/process.c \
    kernel/elf-loader.c \
    kernel/util.c \
    kernel/context.c \
    kernel/scheduler.c \
    kernel/syscall.c \
    kernel/arguments.c \
    kernel/alloc-tracker.c \
    kernel/riscv.c \
    kernel/timer.c \
    "$INITRAMFS_DIR/initramfs.o"

# === Start socket communicatie ===
SOCK=/tmp/hosttime.sock
TIME=$(date +%s)
[ -S "$SOCK" ] && rm "$SOCK"
( echo "$TIME" | socat -d -d - UNIX-LISTEN:"$SOCK",fork ) &
sleep 0.2
    
# === Start QEMU ===
echo "=== Start QEMU ==="
$QEMU -machine virt \
      -m 512M \
      -nographic \
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
      -device virtio-blk-device,drive=drive0,bus=virtio-mmio-bus.1 \
      --no-reboot