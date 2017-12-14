%define SECTORS 3

[org 0x7c00]

; BIOS stores boot drive in dl
mov [BOOT_DRIVE], dl

; clear segment registers
mov ax, 0
mov ds, ax
mov es, ax
mov ss, ax

; set up stack
;   TODO come back to this
mov bp, 0x8000
mov sp, bp

mov bx, STR_TEST
call PrintString

mov bx, 0x7e00
mov cx, 8
call PrintHex
mov bx, STR_LFCR
call PrintString

; target address for disk read (es:bx)
mov ax, 0x7e0
mov es, ax
mov bx, 0x0

; BIOS read sector function
mov ah, 0x02
mov dl, [BOOT_DRIVE] ; drive number
mov ch, 0 ; cylinder 0
mov dh, 0 ; head 0
mov cl, 2 ; sector # (sector idx starts at 1)
mov al, SECTORS ; number of sectors to read

int 0x13

; Check for disk read error
jc disk_error
cmp al, SECTORS
jne disk_error

mov bx, 0x7e00
mov cx, 8
call PrintHex
mov bx, STR_LFCR
call PrintString

mov bx, 0x7f00
mov cx, 8
call PrintHex
mov bx, STR_LFCR
call PrintString

mov bx, 0x8000
mov cx, 8
call PrintHex
mov bx, STR_LFCR
call PrintString

mov bx, 0x8200
mov cx, 8
call PrintHex
mov bx, STR_LFCR
call PrintString

mov bx, 0x8400
mov cx, 8
call PrintHex
mov bx, STR_LFCR
call PrintString

; jmp here forever
jmp $

disk_error:
    mov bx, STR_DISK_ERR
    call PrintString
    jmp $

PrintString:
    pusha
    mov ah, 0x0e
    pstr_loop:
        mov al, [bx]
        add bx, 1
        cmp al, 0
        je pstr_done
        int 0x10
        jmp pstr_loop

    pstr_done:
    popa
    ret

; bx - address to print from
; cx - number of bytes to print
PrintHex:
    pusha
    mov ah, 0x0e
    ; cx bytes -> 2*cx ASCII characters
    shl cx, 1

    phex_loop:
        mov al, [bx]

        mov si, cx
        and si, 1
        jz phex_higher_bits
        and al, 0x0f
        inc bx
        jmp phex_digit_to_ascii
        phex_higher_bits:
        shr al, 4
        
        phex_digit_to_ascii:
        cmp al, 10
        jge phex_digit_letter
        ; 0x30 is ASCII 0
        add al, 0x30
        jmp phex_digit_print

        phex_digit_letter:
        ; 0x41 is ASCII A
        add al, 0x41 - 10

        phex_digit_print:
        int 0x10

        dec cx
        jz phex_done

        jmp phex_loop

    phex_done:
    popa
    ret

; ----- Global variables -----
BOOT_DRIVE: db 0

STR_TEST:
db 'Hello, sailor', 0x0a, 0x0d, 0

STR_DISK_ERR:
db 'Disk read error.', 0x0a, 0x0d, 0

STR_LFCR:
db 0x0a, 0x0d, 0

TESTHEX:
dw 0xcafe, 0xcafe, 0xbabe, 0xface

; pad file up until the 510th byte
times 510 -( $ - $$ ) db 0

dw 0xaa55

; testing data on other sectors
times 256 dw 0xdead
times 256 dw 0xcafe
times 256 dw 0xbabe