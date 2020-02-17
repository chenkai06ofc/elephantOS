%include "boot.s"
%include "gdt.s"

SECTION LOADER vstart=LOADER_BASE_ADDR
jmp loader_start

;---------- GDT table ----------
GDT_BASE: dd 0x00000000, 0x00000000
CODE_DESC: dd 0x0000FFFF, DESC_CODE_HIGH4
DATA_STACK_DESC: dd 0x0000FFFF, DESC_DATA_HIGH4
VIDEO_DESC: dd 0x80000007, DESC_VIDEO_HIGH4 ;limit=(0xbffff-0xb8000)/4k=0x7
;---------- selector definition ----------
SELECTOR_CODE equ (0x0001<<3) + TI_GDT + RPL0
SELECTOR_DATA equ (0x0002<<3) + TI_GDT + RPL0
SELECTOR_VIDEO equ (0x0003<<3) + TI_GDT + RPL0

GDT_SIZE equ $-GDT_BASE
GDT_LIMIT equ GDT_SIZE - 1

gdt_ptr dw GDT_LIMIT
        dd GDT_BASE

msg db "loader is running"
msg_len dw $-msg

; code start here
loader_start:
    ; ---------- clear screen ----------
    call clear_screen

    ; ---------- print message ----------
    mov ax, msg
    mov cx, [msg_len]
    mov dx, 0x0000  ; print at row:0, column:0
    call print_str_at

    ; ---------- load kernel ----------
    push es
    mov ax, KERNEL_BIN_ADDR_SEGMENT
    mov es, ax
    mov bx, KERNEL_BIN_ADDR_OFFSET
    mov eax, KERNEL_START_SECTOR
    mov cx, KERNEL_SECTOR_COUNT
    call read_disk
    pop es

    ; ---------- Steps to enter protected mode ----------
    ; 1. open A20
    ; 2. load GDT
    ; 3. cr0.pe set 1

    ; ---------- open A20 ----------
    in al, 0x92
    or al, 0000_0010b
    out 0x92, al

    ; ---------- load GDT ----------
    lgdt [gdt_ptr]

    ; ---------- cr0.pe set 1 ----------
    mov eax, cr0
    or eax, 0x0000_0001
    mov cr0, eax

    jmp dword SELECTOR_CODE:protected_mode_start

%include "util_print.s"
%include "util_hd.s"

[bits 32]
protected_mode_start:
    mov ax, SELECTOR_DATA
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, LOADER_STACK_TOP
    mov ax, SELECTOR_VIDEO
    mov gs, ax

    mov byte [gs:160], 'P'
    mov byte [gs:161], 0xa4
    jmp $

