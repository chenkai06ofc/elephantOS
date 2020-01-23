; this is a sample loader just print message 'loader start'
%include "boot.s"
SECTION LOADER vstart=LOADER_BASE_ADDR

mov bx, loader_start_message
mov cx, [loader_start_message_len]
mov bp, 160
call print_str

jmp $
loader_start_message db "loader start"
loader_start_message_len dw $-loader_start_message
%include "util.s"