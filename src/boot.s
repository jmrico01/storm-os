/**
 * ---------------------------------------------------------------------------
 * --------------------------- START OF 1ST SECTOR ---------------------------
 * ---------------------------------------------------------------------------
*/
.code16

.set BOOT0_ADDR,    0x7c00
.set BOOT1_ADDR,    0x7e00
.set BOOT1_SECTORS, 1
.set STACK_ADDR,    BOOT0_ADDR

# clear segment registers
xorw %ax, %ax
movw %ax, %ds
movw %ax, %es
movw %ax, %ss

# BIOS stores boot drive in dl
movb %dl, BOOT_DRIVE

# set to normal (80x25 text) video mode
movb $0, %ah
movb $3, %al
int	$0x10

# set up stack
#   TODO come back to this
movw $STACK_ADDR, %bp
movw %bp, %sp

movw $STR_START, %bx
call PrintString16

# target address for disk read - es:bx
movw $0, %ax
movw %ax, %es
movw $BOOT1_ADDR, %bx

# BIOS read sector function
movb $0x02, %ah
movb BOOT_DRIVE, %dl # drive number
movb $0, %ch         # cylinder 0
movb $0, %dh         # head 0
movb $2, %cl         # sector number (sector idx starts at 1)
movb $BOOT1_SECTORS, %al # number of sectors to read

int $0x13

# Check for disk read error
jc disk_error
cmp $BOOT1_SECTORS, %al
jne disk_error

movw $STR_LOADED_BOOT, %bx
call PrintString16

# Jump to BOOT1
jmp BOOT1

jmp .

disk_error:
    movw $STR_DISK_ERROR, %bx
    call PrintString16
    jmp .

BOOT_DRIVE:
    .byte 0

STR_START:
    .ascii "Running boot sector 0.\n\r\0"
STR_LOADED_BOOT:
    .ascii "Loaded full boot sector.\n\r\0"
STR_LOADED_KERNEL:
    .ascii "Loaded kernel.\n\r\0"

STR_DISK_ERROR:
    .ascii "Disk read error.\n\r\0"

PrintString16:
    pusha
    movb $0x0e, %ah
    pstr16_loop:
        movb (%bx), %al
        addw $1, %bx
        cmp $0, %al
        je pstr16_done
        int $0x10
        jmp pstr16_loop

    pstr16_done:
    popa
    ret

# Naive way of padding until 510th byte.
# Makes boot0 have to be < 256 bytes, which is fine I guess.
# TODO: improve this?
.balign 256, 0
.space 254, 0
.word 0xaa55

/**
 * ---------------------------------------------------------------------------
 * --------------------------- START OF 2ND SECTOR ---------------------------
 * ---------------------------------------------------------------------------
*/
.set KERNEL_ADDR,       0x1000
.set KERNEL_SECTORS,    16
.set CODE_SEG,          0x08
.set DATA_SEG,          0x10

BOOT1:

# target address for disk read (es:bx)
movw $0, %ax
movw %ax, %es
movw $KERNEL_ADDR, %bx

# BIOS read sector function
movb $0x02, %ah
movb BOOT_DRIVE, %dl # drive number
movb $0, %ch         # cylinder 0
movb $0, %dh         # head 0
movb $(2 + BOOT1_SECTORS), %cl # sector number (sector idx starts at 1)
movb $KERNEL_SECTORS, %al # number of sectors to read

int $0x13

# Check for disk read error
jc disk_error
cmp $KERNEL_SECTORS, %al
jne disk_error

mov $STR_LOADED_KERNEL, %bx
call PrintString16

# Disable interrupts until we set them up for 32-bit mode
cli
# Register the GDT
lgdt gdt_descriptor

# Switch to 32-bit mode by setting 1st bit of cr0
movl %cr0, %eax
orl  $1, %eax
movl %eax, %cr0

# Issue a long jump to flush the processor instruction pipeline
# This ensures that the 32-bit code flag is truly set
ljmp $CODE_SEG, $protected_mode

# 4-byte alignment for GDT
.balign 4

# -- GDT --
gdt_start:

# First entry of the GDT should be null, to catch errors
gdt_null:
.word 0, 0
.byte 0, 0, 0, 0

# Code segment descriptor
# base = 0, limit = 0xffff
gdt_code:
.word 0xffff        # limit (bits 0-15)
.word 0             # base  (bits 0-15)
.byte 0             # base  (bits 16-23)
.byte 0b10011010    # 1(present) 00(privilege) 1(descriptor type)
                    # 1(code) 0(conforming) 1(readable) 0(accessed)
.byte 0b11001111    # 1(granularity) 1(32-bit default) 0(64-bit seg) 0(AVL)
                    # limit (bits 16-19)
.byte 0             # base  (bits 24-31)

# Data segment descriptor
# base = 0, limit = 0xffff
gdt_data:
.word 0xffff        # limit (bits 0-15)
.word 0             # base  (bits 0-15)
.byte 0             # base  (bits 16-23)
.byte 0b10010010    # 1(present) 00(privilege) 1(descriptor type)
                    # 0(code) 1(expand-down) 1(writable) 0(accessed)
.byte 0b11001111    # 1(granularity) 1(32-bit default) 0(64-bit seg) 0(AVL)
                    # limit (bits 16-19)
.byte 0             # base  (bits 24-31)

gdt_end:

# GDT descriptor
gdt_descriptor:
.word gdt_end - gdt_start - 1   # Size
.long gdt_start                 # Start

# -- 32-bit section, protected mode --
.code32

protected_mode:

# Jump to the kernel code
jmp KERNEL_ADDR

jmp .

# pad file up until the 1024th byte
.balign 1024, 0
