TI_GDT equ 000b
RPL0 equ 00b
SELECTOR_VIDEO equ (0x0003<<3) + TI_GDT + RPL0

ADDR_IO_PORT equ 0x03d4
DATA_IO_PORT equ 0x03d5
IDX_CURSOR_LOCATION_HIGH equ 0x0e
IDX_CURSOR_LOCATION_LOW equ 0x0f

CHAR_ATTR equ 0x07

ASCII_CR equ 0x0d
ASCII_LF equ 0x0a
ASCII_BS equ 0x08
ASCII_SPACE equ 0x20

CHAR_SPACE_WITH_ATTR equ (CHAR_ATTR<<8) + ASCII_SPACE

[bits 32]
section .text
global put_char
global put_str
global put_int
global set_cursor

; -------------------- function put_char --------------------
; use: gs
;   eax, ebx, ecx, edx, esi, edi
put_char:
    ; back registers for ABI
    push ebx
    push esi
    push edi

    mov ax, SELECTOR_VIDEO
    mov gs, ax

    ; ---------- get cursor location ----------
    ; high 8 bits
    mov dx, ADDR_IO_PORT
    mov al, IDX_CURSOR_LOCATION_HIGH
    out dx, al
    mov dx, DATA_IO_PORT
    in al, dx
    mov ah, al
    ; low 8 bits
    mov dx, ADDR_IO_PORT
    mov al, IDX_CURSOR_LOCATION_LOW
    out dx, al
    mov dx, DATA_IO_PORT
    in al, dx

    mov bx, ax ; bx is the cursor location

    mov ecx, [esp + 16] ; get input parameter, which is the char to output
    cmp cl, ASCII_CR
    jz .is_carriage_return
    cmp cl, ASCII_LF
    jz .is_line_feed
    cmp cl, ASCII_BS
    jz .is_backspace
    ; then do .is_other
.is_other:
    shl bx, 1
    mov [gs:bx], cl
    mov byte [gs:bx + 1], CHAR_ATTR
    shr bx, 1
    inc bx
    call .update_cursor
    jmp .exit

.is_backspace:
    dec bx
    shl bx, 1 ; double the bx to match the offset in VRAM
    mov word [gs:bx], CHAR_SPACE_WITH_ATTR
    shr bx, 1
    call _set_cursor
    jmp .exit

; CR & LF is handled together
.is_line_feed:
.is_carriage_return:
    xor dx, dx ; dx = 0, dx:ax is the dividend
    mov ax, bx
    mov si, 80
    div si ; results are in ax, dx
    sub bx, dx
    add bx, 80
    call .update_cursor
    jmp .exit

.exit:
    pop edi
    pop esi
    pop ebx
    ret ; end function call

; function: update cursor position, roll screen if needed
;   parameter: cursor position in bx
.update_cursor:
    cmp bx, 2000
    jl .no_screen_overflow
    call .roll_screen
    mov bx, 1920
.no_screen_overflow:
    call _set_cursor
    ret

; function: move rows: 1~14 to 0~23, fill row 24 with spaces
; use
;   ecx, esi, edi
.roll_screen:
    cld
    mov ecx, 960 ; 80 * 15 - 80 = 1920 chars to move, 2 bytes/per char, 4 bytes/per move, so we need 960 moves
    ; ds, es both start from 0
    mov esi, 0xc00b80a0 ; head of row 1
    mov edi, 0xc00b8000 ; head of row 0
    rep movsd
    ; clear last row
    mov esi, (2000 - 80) * 2 ; head of last row
    mov ecx, 80
.clear_last_row:
    mov word [gs:esi], CHAR_SPACE_WITH_ATTR
    add esi, 2
    loop .clear_last_row
    ret

; -------------------- function set_cursor --------------------
; set cursor location
; use: ax, bx, dx
set_cursor:
    push ebx
    mov ebx, [esp + 8] ; put new cursor location into bx
    call _set_cursor
    pop ebx
    ret

; implementation of set_cursor function, and will be invoked internally by put_char
; function: set cursor location to bx
; arguments
;   bx: new cursor location
; use
;   ax, dx
_set_cursor:
    ; set high 8 bits
    mov dx, ADDR_IO_PORT
    mov al, IDX_CURSOR_LOCATION_HIGH
    out dx, al
    mov dx, DATA_IO_PORT
    mov al, bh
    out dx, al
    ; set low 8 bits
    mov dx, ADDR_IO_PORT
    mov al, IDX_CURSOR_LOCATION_LOW
    out dx, al
    mov dx, DATA_IO_PORT
    mov al, bl
    out dx, al
    ret

; -------------------- function put_str --------------------
; print a char sequence end with 0
; use: ebx, ecx
put_str:
    push ebx
    xor ecx, ecx
    mov ebx, [esp + 8] ; ebx is target string addr
.go_on_put_char:
    mov cl, [ebx]
    cmp cl, 0
    je .end_put_char
    push ecx
    call put_char
    add esp, 4
    inc ebx
    jmp .go_on_put_char
.end_put_char:
    pop ebx
    ret

; -------------------- function put_int --------------------
; use: eax, ecx, edx, ebp, edi
section .data
put_int_buffer db 0,0,0,0,0,0,0,0,0 ; 9 bytes buffer
section .text
put_int:
    push ebp
    push edi
    mov ebp, esp
    mov eax, [ebp + 4*3]

    mov ecx, 8
    mov edi, 7
.copy_to_buffer:
    mov edx, eax
    and edx, 0x0000000f
    cmp dl, 10
    jc .less_than_10
    add dl, 0x37
    jmp .single_copy_to_buffer
.less_than_10:
    add dl, 0x30
.single_copy_to_buffer:
    mov [put_int_buffer + edi], dl
    dec edi
    shr eax, 4
    loop .copy_to_buffer

    push put_int_buffer
    call put_str
    add esp, 4

    mov esp, ebp
    pop edi
    pop ebp
    ret