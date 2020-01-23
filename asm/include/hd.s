; operations related to hd

; eax: LBA sector number
; es:bx: memory start offset to write in
; cx: sector count, (cx < 256)
; use esi, dx, di
read_disk:
    ; write sector count
    mov esi, eax
    mov dx, 0x1F2
    mov ax, cx
    out dx, al
    mov eax, esi

    ; write LBA
    mov di, cx ; back up cx
    mov cl, 8
    ; LBA low
    mov dx, 0x1F3
    out dx, al
    ; LBA mid
    shr eax, cl
    mov dx, 0x1F4
    out dx, al
    ; LBA high
    shr eax, cl
    mov dx, 0x1F5
    out dx, al
    ; device
    shr eax, cl
    and al, 0x0F
    or al, 0xE0
    mov dx, 0x1F6
    out dx, al

    mov cx, di

    ; write command
    mov dx, 0x1F7
    mov al, 0x20
    out dx, al ; start read disk

.not_ready:
    nop
    in al, dx
    and al,0x88
    cmp al,0x08
    jnz .not_ready

    ; start reading from data register
    mov ax, 256
    mul cx
    mov cx, ax
    mov dx, 0x1F0
.go_on_read:
    in ax, dx
    mov [es:bx], ax
    add bx, 2
    jnc .go_on_read_loop_end
    mov ax, es
    add ax, 0x1000
    mov es, ax
.go_on_read_loop_end:
    loop .go_on_read

    ret

