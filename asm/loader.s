%include "boot.s"
%include "gdt.s"

SECTION LOADER vstart=LOADER_BASE_ADDR
LOADER_STACK_TOP equ LOADER_BASE_ADDR
jmp loader_start

; GDT table
GDT_BASE: dd 0x00000000, 0x00000000
CODE_DESC: dd 0x0000FFFF, DESC_CODE_HIGH4
DATA_STACK_DESC: dd 0x0000FFFF, DESC_DATA_HIGH4
VIDEO_DESC: dd 0x80000007, DESC_VIDEO_HIGH4 ;limit=(0xbffff-0xb8000)/4k=0x7
GDT_SIZE equ $-GDT_BASE
GDT_LIMIT equ GDT_SIZE - 1

SELECTOR_CODE equ (0x0001<<3) + TI_GDT + RPL0
SELECTOR_DATA equ (0x0002<<3) + TI_GDT + RPL0
SELECTOR_VIDEO equ (0x0003<<3) + TI_GDT + RPL0

gdt_ptr dw GDT_LIMIT
        dd GDT_BASE
loader_start_msg db "loader start..."
loader_start_msg_len dw $-loader_start_msg

%include "util.s"
%include "hd.s"

loader_start:
; print message
mov bx, loader_start_msg
mov cx, [loader_start_msg_len]
mov bp, 160
call print_str

; ------------------ load kernel ------------------
push es
mov ax, KERNEL_BIN_ADDR_SEGMENT
mov es, ax
mov eax, KERNEL_START_SECTOR
mov bx, KERNEL_BIN_ADDR_OFFSET
mov cx, KERNEL_SECTOR_COUNT
call read_disk
pop es
; ------------------ prepare to enter protected mode ------------------
; 1. open A20
; 2. load GDT
; 3. cr0.pe set 1

; ------------------ open A20 ------------------
in al, 0x92
or al, 0000_0010b
out 0x92, al

; ------------------ load GDT ------------------
lgdt [gdt_ptr]

; ------------------ cr0.pe set 1 ------------------
mov eax, cr0
or eax, 0x0000_0001
mov cr0, eax

jmp $