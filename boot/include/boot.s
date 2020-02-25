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
KERNEL_BIN_ADDR equ 0x70000
KERNEL_BIN_ADDR_SEGMENT equ 0x7000 ; kernel is loaded into 0x70000
KERNEL_BIN_ADDR_OFFSET equ 0
KERNEL_STACK_TOP equ 0xc009_f000

; ---------- page table ----------
PAGE_DIR_TABLE_ADDR equ 0x10_0000
FIRST_PTE_ADDR equ PAGE_DIR_TABLE_ADDR + 0x1000
SECOND_PTE_ADDR equ PAGE_DIR_TABLE_ADDR + 0x2000
PG_P equ 1b
PG_RW_R equ 00b
PG_RW_W equ 10b
PG_US_S equ 000b
PG_US_U equ 100b
