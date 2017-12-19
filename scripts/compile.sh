#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
SRC_DIR="$SCRIPT_DIR/../src"
BUILD_DIR="$SCRIPT_DIR/../build"

GCC_FLAGS="-ffreestanding -m32 -O0 -g3 -gdwarf-2 -fvar-tracking -fvar-tracking-assignments"
GCC_WARNINGS="-Wall -Wno-unused-function"

AS_FLAGS="--32 -march=i386 -g"

LD_FLAGS="-nostdlib -m elf_i386 --nmagic --omagic"

mkdir -p $BUILD_DIR
rm -r -f $BUILD_DIR/*

if [ "$1" = "clean" ]; then
    exit 0
fi

pushd $BUILD_DIR > /dev/null

# Assemble boot sector code
as $AS_FLAGS "$SRC_DIR/boot.s" -o boot.o
ld $LD_FLAGS boot.o -Ttext 0x7c00 -o boot.elf
objcopy --only-keep-debug boot.elf boot.sym
objcopy -S -O binary boot.elf boot.bin
rm -f boot.elf

# Assemble kernel entry
as $AS_FLAGS "$SRC_DIR/kernel_entry.s" -o kernel_entry.o
# Compile main kernel code
gcc $GCC_FLAGS $GCC_WARNINGS -c "$SRC_DIR/kernel.c" -o kernel.o

# Link kernel entry and main kernel code
ld $LD_FLAGS kernel_entry.o kernel.o -Ttext 0x1000 -o kernel.elf
objcopy --only-keep-debug kernel.elf kernel.sym
objcopy -S -O binary kernel.elf kernel.bin
rm -f kernel.elf



dd if=/dev/zero of=kernel.bin bs=1 count=1 seek=$((512*16 - 1))

# Create disk image
cat boot.bin kernel.bin > os_image

# Hacky way of combining debug symbols
# ...somehow it works!
# nevermind, it doesnt. loads the first one only
#cat boot.sym kernel.sym > os_image.sym

popd > /dev/null