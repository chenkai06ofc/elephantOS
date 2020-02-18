%include "boot.s"
SECTION MBR vstart=MBR_START
mov ax, cs ; cs is 0
mov ds, ax
mov es, ax
mov ss, ax
mov fs, ax
mov sp, MBR_START

; ---------- clear screen ----------
call clear_screen

; ---------- print welcome_msg ----------
mov ax, welcome_msg
mov cx, [welcome_msg_len]
mov dx, 0x0000  ; print at row:0, column:0
call print_str_at

; ---------- print loading_msg ----------
mov ax, loading_msg
mov cx, [loading_msg_len]
mov dx, 0x0100  ; print at row:1, column:0
call print_str_at

; ---------- load loader module from hd ----------
mov eax, LOADER_START_SECTOR
mov bx, LOADER_BASE_ADDR
mov cx, LOADER_SECTOR_COUNT
call read_disk

; ---------- print loaded_msg ----------
mov ax, loaded_msg
mov cx, [loaded_msg_len]
mov dx, 0x0200  ; print at row:2, column:0
call print_str_at

; ---------- wait ----------
mov cx, 0x0100
mov dx, 0
mov ah, 0x86
int 0x15

jmp LOADER_BASE_ADDR ; jump to execute loader

%include "util_print.s"
%include "util_hd.s"
welcome_msg db "welcome to elephant OS"
welcome_msg_len dw $-welcome_msg

loading_msg db "loading loader module..."
loading_msg_len dw $-loading_msg

loaded_msg db "loader module is loaded"
loaded_msg_len dw $-loaded_msg

times 510-($-$$) db 0
db 0x55, 0xaa