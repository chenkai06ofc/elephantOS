%include "boot.s"
SECTION MBR vstart=MBR_START
mov ax, cs
mov ds, ax
mov es, ax
mov ss, ax
mov fs, ax
mov sp, 0x7c00

; clear screen
mov ax, 0x600
mov bx, 0x700
mov cx, 0
mov dx, 0x184f
int 0x10

; get cursor
mov ah, 3
mov bh, 0
int 0x10

; print string
mov ax, message
mov bp, ax
mov cx, 11
mov ax, 0x1301
mov bx, 0x2
int 0x10

jmp $
message db "hello world"
times 510-($-$$) db 0
db 0x55, 0xaa