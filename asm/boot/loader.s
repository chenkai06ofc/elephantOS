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


; -------------------- 32-bit mode start --------------------
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

    ; ---------- Steps to enable paging ----------
    ; 1. prepare PDE $ PTE
    ; 2. write Page Dir Table addr to cr3
    ; 3. set cr0.pg
    ; ---------- prepare PDE $ PTE ----------
    call setup_page

    sgdt [gdt_ptr]
    mov ebx, [gdt_ptr + 2]
    or dword [ebx + 0x18 + 4], 0xc000_0000
    add dword [gdt_ptr + 2], 0xc000_0000
    add esp, 0xc000_0000

    ; ---------- write Page Dir Table addr to cr3 ----------
    mov eax, PAGE_DIR_TABLE_ADDR
    mov cr3, eax
    ; ---------- set cr0.pg ----------
    mov eax, cr0
    or eax, 0x8000_0000
    mov cr0, eax

    lgdt [gdt_ptr]
    mov byte [gs:240], 'Y'
    mov byte [gs:241], 0xa4

    jmp $

; function
; put Page Dir Table in 0x100000(1m), and Page Table Entry(PTE) after it
setup_page:
    ; clear space for PDE (4k)
    mov ecx, 1024
    mov esi, 0
.clear_page_dir:
    mov dword [PAGE_DIR_TABLE_ADDR + esi], 0
    inc esi
    loop .clear_page_dir

.create_pde:
    mov eax, PAGE_DIR_TABLE_ADDR | PG_US_U | PG_RW_W | PG_P
    mov [PAGE_DIR_TABLE_ADDR + 4092], eax ; let last PDE point ot Page Dir table itself

    mov eax, FIRST_PTE_ADDR | PG_US_U | PG_RW_W | PG_P
    mov [PAGE_DIR_TABLE_ADDR + 0x0], eax
    mov [PAGE_DIR_TABLE_ADDR + 0xc00], eax

    ; ---------- fill first pte with lowest 1m ----------
    mov ecx, 0x100
    mov esi, 0
    mov edx, PG_US_U | PG_RW_W | PG_P
    mov ebx, FIRST_PTE_ADDR
.fill_1st_pte:
    mov [ebx+esi*4], edx
    add edx, 0x1000
    inc esi
    loop .fill_1st_pte

    ; ---------- create kernel PDE ----------
    mov eax, SECOND_PTE_ADDR | PG_US_U | PG_RW_W | PG_P
    mov ebx, PAGE_DIR_TABLE_ADDR
    mov ecx, 254
    mov esi, 769
.create_kernel_pde:
    mov [ebx+esi*4], eax
    inc esi
    add eax, 0x1000
    loop .create_kernel_pde
    ret