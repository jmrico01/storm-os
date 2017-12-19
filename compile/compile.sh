#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
SRC_DIR="$SCRIPT_DIR/../src"
BUILD_DIR="$SCRIPT_DIR/../build"

GCC_FLAGS="-g -ffreestanding -m32"
GCC_WARNINGS="-Wall -Wno-unused-function"

LD_FLAGS="-nostdlib -m elf_i386 --oformat binary"

if [ "$1" = "clean" ]; then
    rm -r -f build/*

    exit 0
fi

mkdir -p $BUILD_DIR
pushd $BUILD_DIR > /dev/null

# Assemble boot sector code
#nasm "$SRC_DIR/boot.s" -f bin -o boot.bin -l boot.lst
as "$SRC_DIR/boot.s" --32 -march=i386 -o boot.o
ld $LD_FLAGS boot.o -Ttext 0x7c00 -o boot.bin

# Assemble kernel entry
#nasm "$SRC_DIR/kernel_entry.s" -g -f elf -o kernel_entry.o
as "$SRC_DIR/kernel_entry.s" --32 -march=i386 -o kernel_entry.o

# Compile and link main kernel code
gcc $GCC_FLAGS $GCC_WARNINGS -c "$SRC_DIR/kernel.c" -o kernel.o
ld $LD_FLAGS kernel_entry.o kernel.o -Ttext 0x1000 -o kernel.bin

# Create disk image
cat boot.bin kernel.bin > os_image

popd > /dev/null