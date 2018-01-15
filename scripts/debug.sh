#!/bin/bash

gnome-terminal -e "gdb -symbols=./build/kernel.sym -command=./scripts/gdb_commands" &
qemu-system-i386 -s -S -drive format=raw,file=./build/os_image