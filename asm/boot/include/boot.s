; Macro definition file
; ---------- mbr ----------
MBR_START equ 0x7c00

; ---------- loader ----------
LOADER_BASE_ADDR equ 0x900
LOADER_START_SECTOR equ 1
LOADER_SECTOR_COUNT equ 4
LOADER_STACK_TOP equ LOADER_BASE_ADDR

; ---------- kernel ----------
KERNEL_START_SECTOR equ 9
KERNEL_SECTOR_COUNT equ 200
KERNEL_BIN_ADDR_SEGMENT equ 0x7000 ; kernel is loaded into 0x70000
KERNEL_BIN_ADDR_OFFSET equ 0