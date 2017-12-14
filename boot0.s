mov ah, 0x0e
mov al, 'H'
int 0x10
mov al, 'e'
int 0x10
mov al, 'l'
int 0x10
mov al, 'l'
int 0x10
mov al, 'o'
int 0x10
mov al, ' '
int 0x10
mov al, 'S'
int 0x10
mov al, 'a'
int 0x10
mov al, 'i'
int 0x10
mov al, 'l'
int 0x10
mov al, 'o'
int 0x10
mov al, 'r'
int 0x10

; jmp here forever
jmp $

; pad file up until the 510th byte
times 510 -( $ - $$ ) db 0

dw 0xaa55