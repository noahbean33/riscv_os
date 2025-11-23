#!/bin/bash
set -e

echo "======================================="
echo "   Setup Script 🚀"
echo "======================================="

PROJECT_DIR=~/projects/riscv-os64
RISCV_DIR=~/opt/riscv

echo ">>> Step 1: System update"
sudo apt update && sudo apt upgrade -y

echo ">>> Step 2: Install required packages"
sudo apt install -y autoconf automake autotools-dev curl libmpc-dev libmpfr-dev \
    libgmp-dev gawk build-essential bison flex texinfo gperf libtool patchutils \
    bc zlib1g-dev libexpat-dev clang llvm lld make cmake git python3 python3-pip \
    socat qemu-system-misc gdb-multiarch

echo ">>> Step 3: Install VSCode"
sudo snap install --classic code || echo "⚠️  VSCode installation via Snap failed. Install manually."

echo ">>> Step 4: Build RISC-V toolchain"
mkdir -p ~/opt
if [ ! -d "$RISCV_DIR" ]; then
  cd ~
  git clone https://github.com/riscv/riscv-gnu-toolchain.git
  cd riscv-gnu-toolchain
  ./configure --prefix=$RISCV_DIR --with-arch=rv64gc --with-abi=lp64d
  make -j$(nproc)
fi

echo ">>> Step 5: Update PATH"
if ! grep -q "$RISCV_DIR/bin" ~/.bashrc; then
  echo "export PATH=$RISCV_DIR/bin:\$PATH" >> ~/.bashrc
fi
export PATH=$RISCV_DIR/bin:$PATH

echo ">>> Step 6: Create project directory on $PROJECT_DIR"
mkdir -p $PROJECT_DIR
cd $PROJECT_DIR

echo ">>> Step 7: Create a basic directory structure"
mkdir -p .vscode bin boot docs fs include kernel sbi scripts servers/init/include servers/windows/include user/bin user/include user/lib

echo ">>> Step 8: Download OpenSBI"
cd sbi
curl -LO https://github.com/qemu/qemu/raw/v8.0.4/pc-bios/opensbi-riscv64-generic-fw_dynamic.bin
cd ..

echo ">>> Setup complete! 🎉"
echo "Project is in: $PROJECT_DIR"

echo ">>> VSCode is starting..."
code $PROJECT_DIR