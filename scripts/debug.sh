#!/bin/bash

gnome-terminal -e "gdb -symbols=./build/kernel.sym -command=./scripts/gdb_commands" &
qemu-system-i386 -s -S ./build/os_image