; use bios interrupt to get memory capacity & layout
%include "boot.s"
SECTION MBR vstart=MBR_START
mov ax, cs
mov ds, ax
mov es, ax
mov ss, ax
mov fs, ax
mov sp, 0x7c00

; get memory capacity
mov ax, 0xE801
int 0x15
push bx

mov bx, ax
mov bp, 0
call print_nr

pop bx
mov bp, 20
call print_nr

; get memory layout
; si is the start position
mov si, 160
mov ebx, 0
mov di, 0x5000
print_layout:
mov eax, 0xE820
mov ecx, 20
mov edx, 0x534d4150
int 0x15
jc err_occur

cmp ebx, 0
jz print_end
push ebx

mov ebx, [es:di]
mov bp, si
call print_nr_32

mov ebx, [es:di+8]
mov bp, si
add bp, 30
call print_nr_32

mov ebx, [es:di+16]
mov bp, si
add bp, 60
call print_nr_32

pop ebx
add di, 20
add si, 160
jmp print_layout

print_end:
jmp $

err_occur:
mov ax, err_message
mov bp, ax
mov cx, 11
mov ax, 0x1301
mov bx, 0x2
int 0x10
jmp $

%include "util.s"
err_message db "error occurs"
times 510-($-$$) db 0
db 0x55, 0xaa