; function: clear screen
clear_screen:
    mov ax, 0x0600  ; ah = 6, al = 0; 0x06 function of INT 0x10: clear screen
    mov bx, 0x700   ; bh = 0x07, background color is black, foreground color is light gray
    mov cx, 0       ; (cl, ch) = (0, 0), (x, y) of left-up position
    mov dx, 0x184f  ; (dl, dh) = (79, 24), (x, y) of right-down position
    int 0x10
    ret

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