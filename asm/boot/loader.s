%include "boot.s"
SECTION LOADER vstart=LOADER_BASE_ADDR

; ---------- clear screen ----------
call clear_screen

mov ax, msg
mov cx, [msg_len]
mov dx, 0x0000  ; print at row:0, column:0
call print_str_at

jmp $

%include "util_print.s"
msg db "loader is running"
msg_len dw $-msg