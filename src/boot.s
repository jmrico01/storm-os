/**
 * ---------------------------------------------------------------------------
 * --------------------------- START OF 1ST SECTOR ---------------------------
 * ---------------------------------------------------------------------------
*/

.set BOOT0_ADDR,    0x7c00
.set BOOT1_ADDR,    0x7e00
.set BOOT1_SECTORS, 1
.set STACK_ADDR,    BOOT0_ADDR

.set SECTOR_SIZE,   512

.globl start
start:

.code16
cli
cld

# clear segment registers
xorw %ax, %ax
movw %ax, %ds
movw %ax, %es
movw %ax, %ss

# BIOS stores boot drive in dl
movb %dl, BOOT_DRIVE

# set video mode to 80x25 text
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
    .ascii "Running boot sector 0.\r\n\0"
STR_LOADED_BOOT:
    .ascii "Loaded full boot sector.\r\n\0"

STR_DISK_ERROR:
    .ascii "Disk read error.\r\n\0"

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
.set KERNEL_ADDR,       0x100000
.set KERNEL_START,      8
.set KERNEL_SECTORS,    128
.set CODE_SEG,          0x08
.set DATA_SEG,          0x10

.set SMAP_SIG, 0x0534D4150	# "SMAP"

BOOT1:

# enable A20 (copied from assignments)
seta20.1:
inb $0x64, %al
testb $0x2, %al
jnz	seta20.1
movb $0xd1, %al
outb %al, $0x64
seta20.2:
inb	$0x64, %al
testb $0x2, %al
jnz	seta20.2
movb $0xdf, %al
outb %al, $0x60

# detect physical memory map (from assignments)
e820:
xorl    %ebx, %ebx      # ebx must be 0 when first calling e820
movl    $SMAP_SIG, %edx # edx must be 'SMAP' when calling e820
movw    $(smap+4), %di  # set the address of the output buffer
e820.1:
movl    $20, %ecx       # set the size of the output buffer
movl    $0xe820, %eax   # set the BIOS service code
int     $0x15           # call BIOS service e820h
e820.2:
jc	    e820.fail       # error during e820h
cmpl    $SMAP_SIG, %eax # check eax, which should be 'SMAP'
jne	    e820.fail
e820.3:
movl    $20, -4(%di)
addw    $24, %di
cmpl    $0x0, %ebx		# whether it's the last descriptor
je      e820.4
jmp     e820.1
e820.4:					# zero the descriptor after the last one
xorb	%al, %al
movw	$20, %cx
rep	stosb
jmp	e820.done

e820.fail:
movw $STR_E820_ERROR, %bx
call PrintString16
jmp .

e820.done:

# set video mode to graphic 640x480x4
movb $0, %ah
movb $0x12, %al
int $0x10

# Disable interrupts until we set them up for 32-bit mode
# cli
# Register the GDT
lgdt gdt_descriptor

# Switch to 32-bit mode by setting 1st bit of cr0
movl %cr0, %eax
orl  $1, %eax
movl %eax, %cr0

# Issue a long jump to flush the CPU instruction pipeline
# This ensures that the 32-bit code flag is truly set
ljmp $CODE_SEG, $protected_mode

STR_E820_ERROR:
    .ascii "Memory map detection error.\r\n\0"

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

movw $DATA_SEG, %ax
movw	%ax, %ds
movw	%ax, %es
movw	%ax, %fs
movw	%ax, %gs
movw	%ax, %ss

# Load kernel into KERNEL_ADDR, sector by sector
movl $KERNEL_START, %eax
movl $KERNEL_ADDR, %edi

read_sector:
call ReadSectorLBA
addl $1, %eax
addl $SECTOR_SIZE, %edi
cmp $KERNEL_SECTORS, %eax
jl read_sector

# Jump to the kernel code, pass SMAP
movl $smap, %edx
jmp KERNEL_ADDR

jmp .

# eax: logical block address
# edi: address of buffer to write data to
ReadSectorLBA:
    pusha
    # Save LBA in ebx
    movl %eax, %ebx

    # Send bits 24-27 of LBA + some stuff (flags? "drive"?)
    movw $0x1f6, %dx
    shrl $24, %eax
    orb $0b11100000, %al
    outb %al, %dx

    # Send number of sectors
    movw $0x1f2, %dx
    movb $1, %al
    outb %al, %dx

    # Send bits 0-7 of LBA
    movw $0x1f3, %dx
    movl %ebx, %eax
    outb %al, %dx

    # Send bits 8-15 of LBA
    movw $0x1f4, %dx
    movl %ebx, %eax
    shrl $8, %eax
    outb %al, %dx

    # Send bits 16-23 of LBA
    movw $0x1f5, %dx
    movl %ebx, %eax
    shrl $16, %eax
    outb %al, %dx

    # Send command read with retry
    movw $0x1f7, %dx
    movb $0x20, %al
    outb %al, %dx

    readlba_wait:
    inb %dx, %al
    testb $8, %al
    jz readlba_wait

    # 256 words = 1 sector
    mov $(SECTOR_SIZE / 2), %eax
    # Multiply %eax by # of sectors to read
    # xor %ebx, %ebx
    # mov %cl, %bl
    # mul %ebx

    mov %eax, %ecx
    mov $0x1f0, %edx
    rep insw

    popa
    ret

# reserve space for memory map
smap:
.space 0xc00, 0

# pad file up until the 4096th byte
.balign 1024, 0
