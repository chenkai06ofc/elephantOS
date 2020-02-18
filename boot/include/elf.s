PT_NULL             equ 0
; ---------- elf header ----------
E_PHOFF_OFFSET      equ 28
E_PHENTSIZE_OFFSET  equ 42
E_PHNUM_OFFSET      equ 44

; ---------- program header ----------
P_TYPE_OFFSET       equ 0
P_OFFSET_OFFSET     equ 4
P_VADDR_OFFSET      equ 8
P_FILESZ_OFFSET     equ 16