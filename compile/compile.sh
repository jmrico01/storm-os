#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
SRC_DIR="$SCRIPT_DIR/../src"
BUILD_DIR="$SCRIPT_DIR/../build"

mkdir -p $BUILD_DIR
pushd $BUILD_DIR > /dev/null

# Assemble boot sector code
nasm "$SRC_DIR/boot.s" -f bin -o boot.bin

# Assemble kernel entry
nasm "$SRC_DIR/kernel_entry.s" -f elf -o kernel_entry.o

# Compile and link main kernel code
gcc -ffreestanding -m32 -c "$SRC_DIR/kernel.c" -o kernel.o
ld -o kernel.bin -m elf_i386 -Ttext 0x1000 kernel_entry.o kernel.o --oformat binary

# Create disk image
cat boot.bin kernel.bin > os_image

popd > /dev/null