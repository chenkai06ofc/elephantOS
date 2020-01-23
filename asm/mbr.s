%include "boot.s"
SECTION MBR vstart=MBR_START
mov ax, cs
mov ds, ax
mov es, ax
mov ss, ax
mov fs, ax
mov sp, MBR_START

; print message
mov bx, mbr_message
mov cx, [mbr_message_len]
mov bp, 0
call print_str

; load loader from hd
mov eax, LOADER_START_SECTOR
mov bx, LOADER_BASE_ADDR
mov cx, LOADER_SECTOR_COUNT
call read_disk

jmp LOADER_BASE_ADDR

mbr_message db "mbr start..."
mbr_message_len dw $-mbr_message
%include "util.s"
%include "hd.s"
times 510-($-$$) db 0
db 0x55, 0xaa