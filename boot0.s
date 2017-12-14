[org 0x7c00]

; BIOS stores boot drive in dl
mov [BOOT_DRIVE], dl

; clear segment registers
xor ax, ax
mov ds, ax
mov es, ax
mov ss, ax

; set up stack
;   TODO come back to this
mov bp, 0x8000
mov sp, bp

;mov bx, TESTSTR
;call PrintString

; BIOS read sector function
mov ah, 0x02
mov dl, [BOOT_DRIVE] ; drive number
mov ch, 0 ; cylinder 0
mov dh, 0 ; track 0
mov cl, 2 ; 2nd sector (sector count starts at 1)
mov al, 2 ; number of sectors to read

; target address (es:bx)
mov bx, 0x9000

int 0x13

jc disk_error
cmp al, 1
jne disk_error

mov bx, TESTSTR
call PrintString

; jmp here forever
jmp $

disk_error:
    mov bx, DISK_ERR_STR
    call PrintString
    jmp $

PrintString:
    pusha
    mov ah, 0x0e
    print_loop:
        mov al, [bx]
        add bx, 1
        cmp al, 0
        je print_done
        int 0x10
        jmp print_loop

    print_done:
        popa
        ret

; ----- Global variables -----
BOOT_DRIVE: db 0

TESTSTR:
db 'Hello, sailor', 0

DISK_ERR_STR:
db 'Disk read error.', 0

; pad file up until the 510th byte
times 510 -( $ - $$ ) db 0

dw 0xaa55

; testing data on other sectors
times 256 dw 0xcafe
times 256 dw 0xbabe
times 256 dw 0xdead
times 256 dw 0xface