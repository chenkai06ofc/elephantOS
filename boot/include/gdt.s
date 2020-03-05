; Macro definition file
;---------- gdt descriptor attributes ----------

; arguments
;   1:BASE, 2:LIMIT, 3:G, 4:D, 5:P, 6:DPL, 7:S, 8:TYPE
%macro DD_SEG_DESC_HIGH 8
    dd (%1 & 0xff000000) + (%3 << 23) + (%4 << 22) + (%2 & 0xff0000) + (%5 << 15) +\
    (%6 << 13) + (%7 << 12) + (%8 << 8) + ((%1 & 0xff0000) >> 16)
%endmacro
; arguments
;   1:BASE, 2:LIMIT
%macro DD_SEG_DESC_LOW 2
    dd ((%1 & 0x0000ffff) << 16) + (%2 & 0x0000ffff)
%endmacro

;---------- selector attribute ----------
RPL0 equ 00b
RPL1 equ 01b
RPL2 equ 10b
RPL3 equ 11b
TI_GDT equ 000b
TI_LDT equ 100b
