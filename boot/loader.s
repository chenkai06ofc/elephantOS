%include "boot.s"
%include "gdt.s"
%include "elf.s"

SECTION LOADER vstart=LOADER_BASE_ADDR
jmp loader_start

;---------- GDT table ----------
; DD_SEG_DESC_LOW   1:BASE, 2:LIMIT
; DD_SEG_DESC_HIGH  1:BASE, 2:LIMIT, 3:G, 4:D, 5:P, 6:DPL, 7:S, 8:TYPE
GDT_BASE:           dd 0x00000000, 0x00000000
K_CODE_DESC:        DD_SEG_DESC_LOW 0, 0xfffff
                    DD_SEG_DESC_HIGH 0, 0xfffff, 1, 1, 1, 0, 1, 1000b
K_DATA_STACK_DESC:  DD_SEG_DESC_LOW 0, 0xfffff
                    DD_SEG_DESC_HIGH 0, 0xfffff, 1, 1, 1, 0, 1, 0010b
VIDEO_DESC:         DD_SEG_DESC_LOW 0xb8000, 7 ;limit=(0xbffff-0xb8000)/4k=0x7
                    DD_SEG_DESC_HIGH 0xb8000, 7, 1, 1, 1, 0, 1, 0010b
TSS_DESC:           DD_SEG_DESC_LOW 0, 103
                    DD_SEG_DESC_HIGH 0, 103, 1, 0, 1, 0, 0, 1001b ; not busy, if busy TYPE will be 1011b
U_CODE_DESC:        DD_SEG_DESC_LOW 0, 0xfffff
                    DD_SEG_DESC_HIGH 0, 0xfffff, 1, 1, 1, 3, 1, 1000b
U_DATA_STACK_DESC:  DD_SEG_DESC_LOW 0, 0xfffff
                    DD_SEG_DESC_HIGH 0, 0xfffff, 1, 1, 1, 3, 1, 0010b

GDT_SIZE equ $-GDT_BASE
GDT_LIMIT equ GDT_SIZE - 1

;---------- selector definition ----------
SELECTOR_K_CODE equ (0x0001<<3) + TI_GDT + RPL0
SELECTOR_K_DATA equ (0x0002<<3) + TI_GDT + RPL0
SELECTOR_VIDEO equ (0x0003<<3) + TI_GDT + RPL0

; video desc for use after entering protected mode
DESC_VIR_VIDEO_LOW: DD_SEG_DESC_LOW 0xc00b_8000, 7
DESC_VIR_VIDEO_HIGH: DD_SEG_DESC_HIGH 0xc00b_8000, 7, 1, 1, 1, 0, 1, 0010b

gdt_ptr:    dw GDT_LIMIT
            dd GDT_BASE

msg: db "loader is running..."
msg_len: dw $-msg

; code start here
loader_start:
    ; ---------- clear screen ----------
    call clear_screen

    ; ---------- get memory volume ----------
    mov eax, 0x88
    int 0x15
    add ax, 0x400 ; the return volume doesn't include the low 1mb
    shl eax, 10
    mov [0x800], eax

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

    jmp dword SELECTOR_K_CODE:protected_mode_start

%include "util_print.s"
%include "util_hd.s"


; -------------------- 32-bit mode start --------------------
[bits 32]
protected_mode_start:
    mov ax, SELECTOR_K_DATA
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, LOADER_STACK_TOP
    mov ax, SELECTOR_VIDEO
    mov gs, ax

    ; ---------- prepare to reload gdt ----------
    sgdt [gdt_ptr]
    mov eax, [DESC_VIR_VIDEO_LOW]
    mov [VIDEO_DESC], eax
    mov eax, [DESC_VIR_VIDEO_HIGH]
    mov [VIDEO_DESC + 4], eax
    add dword [gdt_ptr + 2], 0xc000_0000

    ; ---------- Steps to enable paging ----------
    ; 1. prepare PDE $ PTE
    ; 2. write Page Dir Table addr to cr3
    ; 3. set cr0.pg
    ; ---------- prepare PDE $ PTE ----------
    call setup_page
    add esp, 0xc000_0000
    ; ---------- write Page Dir Table addr to cr3 ----------
    mov eax, PAGE_DIR_TABLE_ADDR
    mov cr3, eax
    ; ---------- set cr0.pg ----------
    mov eax, cr0
    or eax, 0x8000_0000
    mov cr0, eax

    ; reload gdt
    lgdt [gdt_ptr]

    mov ax, SELECTOR_VIDEO
    mov gs, ax

enter_kernel:
    call kernel_init
    mov esp, KERNEL_STACK_TOP
    jmp eax

    jmp $

; -------------------- function setup_page --------------------
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

; -------------------- function kernel_init --------------------
; return value
;   eax: kernel entry point
kernel_init:
    xor ecx, ecx
    xor edx, edx

    mov dx, [KERNEL_BIN_ADDR + E_PHENTSIZE_OFFSET] ; dx is program header size
    mov ebx, [KERNEL_BIN_ADDR + E_PHOFF_OFFSET] ; ebx is e_phoff
    add ebx, KERNEL_BIN_ADDR ; now ebx is addr of 1st program header
    mov cx, [KERNEL_BIN_ADDR + E_PHNUM_OFFSET]

; use ebx as addr of each program header
.each_segment:
    cmp byte [ebx + P_TYPE_OFFSET], PT_NULL
    je .if_pt_null
    push ecx
    mov ecx, [ebx + P_FILESZ_OFFSET]
    mov esi, [ebx + P_OFFSET_OFFSET]
    add esi, KERNEL_BIN_ADDR
    mov edi, [ebx + P_VADDR_OFFSET]
    call mem_cpy
    pop ecx
.if_pt_null:
    add ebx, edx
    loop .each_segment
    mov eax, [KERNEL_BIN_ADDR + E_ENTRY_OFFSET] ; eax is e_entry
    ret

; -------------------- function mem_cpy --------------------
; arguments
;   edi: dst, esi: src, ecx: size
mem_cpy:
    cld
    rep movsb
    ret