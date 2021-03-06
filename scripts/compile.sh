#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
SRC_DIR="$SCRIPT_DIR/../src"
BUILD_DIR="$SCRIPT_DIR/../build"

GCC_FLAGS="-std=gnu99 -nostdinc -ffreestanding -m32 -O0 -g3 -gdwarf-2 -fno-stack-protector -fvar-tracking -fvar-tracking-assignments"
GCC_WARNINGS="-Wall -Wextra -Wno-unused-function -Wno-unused-parameter -Wno-type-limits"

AS_FLAGS="--32 -march=i386 -g"

LD_FLAGS="-nostdlib -m elf_i386 -e start --nmagic --omagic"

mkdir -p $BUILD_DIR
rm -r -f $BUILD_DIR/*

if [ "$1" = "clean" ]; then
    exit 0
fi

pushd $BUILD_DIR > /dev/null

# Assemble boot loader code
as $AS_FLAGS "$SRC_DIR/boot.s" -o boot.o
ld $LD_FLAGS boot.o -Ttext 0x7c00 -o boot.elf
objcopy --only-keep-debug boot.elf boot.sym
objcopy --strip-debug -O binary boot.elf boot.bin
rm -f boot.elf
echo "boot.bin size is " `stat --printf="%s" boot.bin`


# Assemble kernel entry
as $AS_FLAGS "$SRC_DIR/kernel_entry.s" -o kernel_entry.o
# Compile main kernel code
gcc $GCC_FLAGS $GCC_WARNINGS -c "$SRC_DIR/kernel.c" -o kernel.o
gcc $GCC_FLAGS $GCC_WARNINGS -c "$SRC_DIR/ctx_switch.s" -o kernel_cs.o
gcc $GCC_FLAGS $GCC_WARNINGS -c "$SRC_DIR/trap_main.S" -o kernel_trap.o
# gcc $GCC_FLAGS $GCC_WARNINGS "$SRC_DIR/ctx_switch.s" "$SRC_DIR/trap_main.S" -o kernel_asm.o

# Link kernel entry and main kernel code
ld $LD_FLAGS kernel_entry.o kernel.o kernel_cs.o kernel_trap.o \
    -T$SCRIPT_DIR/linker.ld -Ttext 0x100000 -o kernel.elf
objcopy --only-keep-debug kernel.elf kernel.sym
objcopy --strip-debug -O binary kernel.elf kernel.bin
rm -f kernel.elf
echo "kernel.bin size is " `stat --printf="%s" kernel.bin`
dd if=/dev/zero of=kernel.bin bs=1 count=1 seek=$((512*248 - 1))


GCC_FLAGS_USER="-std=gnu99 -nostdinc -m32 -O0 -g3 -gdwarf-2 -fno-stack-protector"
LD_FLAGS_USER="-nostdlib -e start -m elf_i386"

# Compile user libraries
gcc $GCC_FLAGS_USER $GCC_WARNINGS -c "$SRC_DIR/user/lib/syscall.c" -o user_libs.o

# Assemble user entry
as $AS_FLAGS "$SRC_DIR/user/entry.s" -o user_entry.o
# Compile user code
gcc $GCC_FLAGS_USER $GCC_WARNINGS -c "$SRC_DIR/user/user.c" -o pong.o
ld $LD_FLAGS_USER user_libs.o user_entry.o pong.o -Ttext 0x40000000 -o pong.elf
echo "pong.elf size is " `stat --printf="%s" pong.elf`
dd if=/dev/zero of=pong.elf bs=1 count=1 seek=$((512*64 - 1))


# Create disk image
cat boot.bin kernel.bin pong.elf > os_image

# Hacky way of combining debug symbols
# ...somehow it works!
# nevermind, it doesn't. loads the first one only
#cat boot.sym kernel.sym > os_image.sym

#cat boot.sym > os_image.sym
cat kernel.sym > os_image.sym

popd > /dev/null