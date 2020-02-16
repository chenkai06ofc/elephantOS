; function: print string at a given cursor position
; arguments:
; (dh, dl): (row, column) of cursor position
; ax: string address, cx: string len
print_str_at:
    ; set cursor position
    push ax
    push cx
    mov ah, 2       ; 0x02 function of INT 0x10: set cursor position
    mov bh, 0       ; page number
    int 0x10
    ; print string
    pop cx
    pop ax
    mov bp, ax
    mov ah, 0x13    ; 0x13 function of INT 0x10: write string
    mov al, 0x01    ; write mode is 01, update cursor after writing
    mov bx, 0x07    ; bh = 0: page number; bl = 7: background black, foreground light gray
    int 0x10
    ret