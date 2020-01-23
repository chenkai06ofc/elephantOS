; parameter
;   bx: string address,
;   cx: string length
;   bp: display start position
; use ax, gs
print_str:
    mov ax, 0xb800
    mov gs, ax
print_char:
    mov al, [bx]
    mov [gs:bp], al
    mov byte [gs:bp+1], 0x07
    add bp, 2
    inc bx
    loop print_char
    ret
; end print_str


LINE_BP_COUNT equ 160
; print number as hex, the input number is put in register bx
; parameter
;   bx: input number
;   bp: display start position
; use ax, gs, cx
print_nr:
    mov ax, 0xb800
    mov gs, ax
    mov byte [gs:bp], '0'
    mov byte [gs:bp+1], 0x07
    mov byte [gs:bp+2], 'x'
    mov byte [gs:bp+3], 0x07
    add bp, 4

    mov cx, 4
print_single_nr:
    mov ax, bx
    and ax, 0xF000
    shr ax, 12
    cmp ax, 10
    jc less_than_10
    add al, ('A' - 10)
    jmp cmp_end
less_than_10:
    add al, '0'
cmp_end:

    mov [gs:bp], al
    mov byte [gs:bp+1], 0x07
    add bp, 2
    shl bx, 4
    loop print_single_nr
    ret
; end print_nr


; print number as hex, the input number is put in register ebx
; parameter
;   ebx: input number
;   bp: display start position
; use eax, gs, cx
print_nr_32:
    mov ax, 0xb800
    mov gs, ax
    mov byte [gs:bp], '0'
    mov byte [gs:bp+1], 0x07
    mov byte [gs:bp+2], 'x'
    mov byte [gs:bp+3], 0x07
    add bp, 4

    mov cx, 8
print_single_nr_32:
    mov eax, ebx
    and eax, 0xF0000000
    shr eax, 28

    cmp ax, 10
    jc less_than_10_32
    add al, ('A' - 10)
    jmp cmp_end_32
less_than_10_32:
    add al, '0'
cmp_end_32:

    mov [gs:bp], al
    mov byte [gs:bp+1], 0x07
    add bp, 2
    shl ebx, 4
    loop print_single_nr_32
    ret
; end print_nr_32
