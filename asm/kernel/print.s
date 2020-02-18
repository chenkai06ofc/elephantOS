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
put_char:
    pushad ; back all registers
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

    mov ecx, [esp + 36] ; get input parameter, which is the char to output
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
    call .set_cursor
    ret

.is_backspace:
    dec bx
    shl bx, 1 ; double the bx to match the offset in VRAM
    mov word [gs:bx], CHAR_SPACE_WITH_ATTR
    shr bx, 1
    call .set_cursor
    ret

; CR & LF is handled together
.is_line_feed:
.is_carriage_return:
    xor dx, dx ; dx = 0, dx:ax is the dividend
    mov ax, bx
    mov si, 80
    div si ; results are in ax, dx
    sub bx, dx
    add bx, 80
    cmp bx, 2000
    jl .no_screen_overflow
.screen_overflow:
    call .roll_screen
    mov bx, 1920
.no_screen_overflow:
    call .set_cursor
    ret



; function: set cursor location to bx
; arguments
;   bx: new cursor location
; use
;   ax, dx
.set_cursor:
    ; set high 8 bits
    mov dx, ADDR_IO_PORT
    mov al, IDX_CURSOR_LOCATION_HIGH
    out dx, al
    mov dx, DATA_IO_PORT
    mov al, bh
    out dx, alggit
    ; set low 8 bits
    mov dx, ADDR_IO_PORT
    mov al, IDX_CURSOR_LOCATION_LOW
    out dx, al
    mov dx, DATA_IO_PORT
    mov al, bl
    out dx, al
    ret

; function: move rows: 1~14 to 0~23, fill row 24 with spaces
; use
;   ecx, esi, edi
.roll_screen:
    cld
    mov ecx, 960 ; 80 * 15 - 80 = 1920 chars to move, 2 bytes/per char, 4 bytes/per move, so we need 960 moves
    ; ds, es both start from 0
    mov esi, 0xb80a0 ; head of row 1
    mov edi, 0xb8000 ; head of row 0
    rep movsd
    ; clear last row
    mov esi, (2000 - 80) * 2 ; head of last row
    mov ecx, 80
.clear_last_row:
    mov word [gs:esi], CHAR_SPACE_WITH_ATTR
    add esi, 2
    loop .clear_last_row
    ret