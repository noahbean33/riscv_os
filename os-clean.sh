# === Directories ===
INITRAMFS_DIR=fs
KERNEL_OUT_DIR=bin

# === Cleanup ===
rm -f user/lib/*.a
rm -f user/bin/*.o user/bin/crt0.o user/bin/init_crt0.o
rm -f user/*.elf user/*.map
rm -f "$INITRAMFS_DIR"/initramfs.tar "$INITRAMFS_DIR"/initramfs.o
rm -f "$KERNEL_OUT_DIR"/kernel.elf "$KERNEL_OUT_DIR"/kernel.map