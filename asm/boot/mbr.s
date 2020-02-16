%include "boot.s"
SECTION MBR vstart=MBR_START
mov ax, cs ; cs is 0
mov ds, ax
mov es, ax
mov ss, ax
mov fs, ax
mov sp, MBR_START

; ---------- clear screen ----------
mov ax, 0x0600  ; ah = 6, al = 0; 0x06 function of INT 0x10: clear screen
mov bx, 0x700   ; bh = 0x07, background color is black, foreground color is light gray
mov cx, 0       ; (cl, ch) = (0, 0), (x, y) of left-up position
mov dx, 0x184f  ; (dl, dh) = (79, 24), (x, y) of right-down position
int 0x10

; ---------- print welcome_msg ----------
mov ax, welcome_msg
mov cx, [welcome_msg_len]
mov dx, 0x0000  ; print at row:0, column:0
call print_str_at

; ---------- print loading_msg ----------
mov ax, loading_msg
mov cx, [loading_msg_len]
mov dx, 0x0100  ; print at row:1, column:0
call print_str_at

; ---------- print loaded_msg ----------
mov ax, loaded_msg
mov cx, [loaded_msg_len]
mov dx, 0x0200  ; print at row:2, column:0
call print_str_at

jmp $

; function: print string at a given cursor position
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

welcome_msg db "welcome to elephant OS"
welcome_msg_len dw $-welcome_msg

loading_msg db "loading loader module..."
loading_msg_len dw $-loading_msg

loaded_msg db "loader module is loaded"
loaded_msg_len dw $-loaded_msg

times 510-($-$$) db 0
db 0x55, 0xaa