[bits 32]
%define ERROR_CODE nop
%define NO_ERROR_CODE push 0

extern put_str
extern intr_handler_list

section .data
intr_msg db "interrupt occur!", 0xa, 0

global intr_entry_list
intr_entry_list:

%macro INTR_ENTRY 2
section .text
intr%1entry:
    %2
    push ds
    push es
    push gs
    push fs
    pushad

    ; send EOI to PIC
    mov al, 0x20
    out 0xa0, al
    out 0x20, al

    ;push esp
    push %1
    call [intr_handler_list + %1*4]
    add esp, 4

    popad
    pop fs
    pop gs
    pop es
    pop ds

    add esp, 4 ; skip error_code
    iret
section .data
    dd intr%1entry
%endmacro

INTR_ENTRY 0x00, NO_ERROR_CODE
INTR_ENTRY 0x01, NO_ERROR_CODE
INTR_ENTRY 0x02, NO_ERROR_CODE
INTR_ENTRY 0x03, NO_ERROR_CODE
INTR_ENTRY 0x04, NO_ERROR_CODE
INTR_ENTRY 0x05, NO_ERROR_CODE
INTR_ENTRY 0x06, NO_ERROR_CODE
INTR_ENTRY 0x07, NO_ERROR_CODE
INTR_ENTRY 0x08, ERROR_CODE
INTR_ENTRY 0x09, NO_ERROR_CODE
INTR_ENTRY 0x0a, ERROR_CODE
INTR_ENTRY 0x0b, ERROR_CODE
INTR_ENTRY 0x0c, ERROR_CODE
INTR_ENTRY 0x0d, ERROR_CODE
INTR_ENTRY 0x0e, ERROR_CODE
INTR_ENTRY 0x0f, NO_ERROR_CODE
INTR_ENTRY 0x10, NO_ERROR_CODE
INTR_ENTRY 0x11, ERROR_CODE
INTR_ENTRY 0x12, NO_ERROR_CODE
INTR_ENTRY 0x13, NO_ERROR_CODE
INTR_ENTRY 0x14, NO_ERROR_CODE
INTR_ENTRY 0x15, NO_ERROR_CODE
INTR_ENTRY 0x16, NO_ERROR_CODE
INTR_ENTRY 0x17, NO_ERROR_CODE
INTR_ENTRY 0x18, NO_ERROR_CODE
INTR_ENTRY 0x19, NO_ERROR_CODE
INTR_ENTRY 0x1a, NO_ERROR_CODE
INTR_ENTRY 0x1b, NO_ERROR_CODE
INTR_ENTRY 0x1c, NO_ERROR_CODE
INTR_ENTRY 0x1d, NO_ERROR_CODE
INTR_ENTRY 0x1e, NO_ERROR_CODE
INTR_ENTRY 0x1f, NO_ERROR_CODE
INTR_ENTRY 0x20, NO_ERROR_CODE