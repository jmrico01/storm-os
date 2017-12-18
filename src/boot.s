; -----------------------------------------------------------------------------
; ---------------------------- START OF 1ST SECTOR ----------------------------
; -----------------------------------------------------------------------------
[bits 16]

BOOT0_ADDR      equ 0x7c00
BOOT1_ADDR      equ 0x7e00
BOOT1_SECTORS   equ 1
STACK_ADDR      equ BOOT0_ADDR - 4

[org BOOT0_ADDR]

; BIOS stores boot drive in dl
mov [BOOT_DRIVE], dl

; clear segment registers
mov ax, 0
mov ds, ax
mov es, ax
mov ss, ax

; set to normal (80x25 text) video mode */
mov ah, 0
mov al, 3
int	0x10

; set up stack
;   TODO come back to this
mov bp, STACK_ADDR
mov sp, bp

mov bx, STR_START
call PrintString16

; target address for disk read (es:bx)
mov ax, 0
mov es, ax
mov bx, BOOT1_ADDR

; BIOS read sector function
mov ah, 0x02
mov dl, [BOOT_DRIVE] ; drive number
mov ch, 0 ; cylinder 0
mov dh, 0 ; head 0
mov cl, 2 ; sector # (sector idx starts at 1)
mov al, BOOT1_SECTORS ; number of sectors to read

int 0x13

; Check for disk read error
jc disk_error
cmp al, BOOT1_SECTORS
jne disk_error

mov bx, STR_LOADED_BOOT
call PrintString16

; Jump to BOOT1
jmp BOOT1

; jmp here forever
jmp $

disk_error:
    mov bx, STR_DISK_ERR
    call PrintString16
    jmp $

PrintString16:
    pusha
    mov ah, 0x0e
    pstr16_loop:
        mov al, [bx]
        add bx, 1
        cmp al, 0
        je pstr16_done
        int 0x10
        jmp pstr16_loop

    pstr16_done:
    popa
    ret

; bx - address to print from
; cx - number of bytes to print
PrintHex16:
    pusha
    mov ah, 0x0e
    ; cx bytes -> 2*cx ASCII characters
    shl cx, 1

    phex16_loop:
        mov al, [bx]

        mov si, cx
        and si, 1
        jz phex16_higher_bits
        and al, 0x0f
        inc bx
        jmp phex16_digit_to_ascii
        phex16_higher_bits:
        shr al, 4
        
        phex16_digit_to_ascii:
        cmp al, 10
        jge phex16_digit_letter
        ; 0x30 is ASCII 0
        add al, 0x30
        jmp phex16_digit_print

        phex16_digit_letter:
        ; 0x41 is ASCII A
        add al, 0x41 - 10

        phex16_digit_print:
        int 0x10

        dec cx
        jz phex16_done

        jmp phex16_loop

    phex16_done:
    popa
    ret

; ----- Global variables -----
BOOT_DRIVE:
db 0

STR_START:
db 'Running boot sector.', 0x0a, 0x0d, 0
STR_LOADED_BOOT:
db 'Loaded extended boot sector.', 0x0a, 0x0d, 0
STR_LOADED_KERNEL:
db 'Loaded kernel.', 0x0a, 0x0d, 0

STR_DISK_ERR:
db 'Disk read error.', 0x0a, 0x0d, 0

STR_LFCR:
db 0x0a, 0x0d, 0

TESTHEX:
dw 0xcafe, 0xcafe, 0xbabe, 0xface

; pad file up until the 510th byte
times 510 -( $ - $$ ) db 0

dw 0xaa55

; -----------------------------------------------------------------------------
; ---------------------------- START OF 2ND SECTOR ----------------------------
; -----------------------------------------------------------------------------
KERNEL_ADDR     equ 0x1000
KERNEL_SECTORS  equ 1
CODE_SEG        equ gdt_code - gdt_start
DATA_SEG        equ gdt_data - gdt_start

BOOT1:

; target address for disk read (es:bx)
mov ax, 0
mov es, ax
mov bx, KERNEL_ADDR

; BIOS read sector function
mov ah, 0x02
mov dl, [BOOT_DRIVE] ; drive number
mov ch, 0 ; cylinder 0
mov dh, 0 ; head 0
mov cl, 3 ; sector # (sector idx starts at 1)
mov al, KERNEL_SECTORS ; number of sectors to read

int 0x13

; Check for disk read error
jc disk_error
cmp al, KERNEL_SECTORS
jne disk_error

mov bx, STR_LOADED_KERNEL
call PrintString16

; Register the GDT
lgdt [gdt_descriptor]
; Disable interrupts until we set them up for 32-bit mode
cli

; Switch to 32-bit mode by setting 1st bit of cr0
mov eax, cr0
or eax, 1
mov cr0, eax

; Issue a long jump to flush the processor instruction pipeline
jmp CODE_SEG:protected_mode

; -- GDT --
gdt_start:

; First entry of the GDT should be null, to catch errors
gdt_null:
dd 0
dd 0

; Code segment descriptor
; base = 0, limit = 0xffff
gdt_code:
dw 0xffff       ; limit (bits 0-15)
dw 0            ; base  (bits 0-15)
db 0            ; base  (bits 16-23)
db 10011010b    ; 1(present) 00(privilege) 1(descriptor type)
                ; 1(code) 0(conforming) 1(readable) 0(accessed)
db 11001111b    ; 1(granularity) 1(32-bit default) 0(64-bit seg) 0(AVL)
                ; limit (bits 16-19)
db 0            ; base  (bits 24-31)

; Data segment descriptor
; base = 0, limit = 0xffff
gdt_data:
dw 0xffff       ; limit (bits 0-15)
dw 0            ; base  (bits 0-15)
db 0            ; base  (bits 16-23)
db 10010010b    ; 1(present) 00(privilege) 1(descriptor type)
                ; 0(code) 1(expand-down) 1(writable) 0(accessed)
db 11001111b    ; 1(granularity) 1(32-bit default) 0(64-bit seg) 0(AVL)
                ; limit (bits 16-19)
db 0            ; base  (bits 24-31)

gdt_end:

; GDT descriptor
gdt_descriptor:
dw gdt_end - gdt_start - 1  ; Size
dd gdt_start                ; Start

; -- 32-bit section, protected mode --
[bits 32]

VIDEO_MEMORY        equ 0xb8000
CHAR_WHITE_ON_BLACK equ 0x0f

protected_mode:

; Jump to the kernel code
jmp KERNEL_ADDR

jmp $

; pad file up until the 1024th byte
times 1024 -( $ - $$ ) db 0