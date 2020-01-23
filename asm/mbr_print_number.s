%include "boot.s"
SECTION MBR vstart=MBR_START
mov ax, cs
mov ds, ax
mov es, ax
mov ss, ax
mov fs, ax
mov sp, 0x7c00

mov ebx, 0xAC4534E8
mov bp, LINE_BP_COUNT
call print_nr_32

jmp $
%include "util.s"
times 510-($-$$) db 0
db 0x55, 0xaa