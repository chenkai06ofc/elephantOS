#ifndef __KERNEL_GLOBAL_H
#define __KERNEL_GLOBAL_H

#define RPL0 0
#define RPL1 1
#define RPL2 2
#define RPL3 3

#define TI_GDT 0
#define TI_LDT 1

#define SELECTOR_K_CODE     ((1 << 3) + (TI_GDT << 2) + RPL0)
#define SELECTOR_K_DATA     ((2 << 3) + (TI_GDT << 2) + RPL0)
#define SELECTOR_K_STACK    SELECTOR_K_DATA
#define SELECTOR_TSS        ((4 << 3) + (TI_GDT << 2) + RPL0)
#define SELECTOR_U_CODE     ((5 << 3) + (TI_GDT << 2) + RPL3)
#define SELECTOR_U_DATA     ((6 << 3) + (TI_GDT << 2) + RPL3)
#define SELECTOR_U_STACK    SELECTOR_U_DATA

#define IDT_P 1
#define IDT_DPL0 0
#define IDT_DPL3 3
#define IDT_INTR_32 0xE
#define IDT_TRAP_32 0xF

#define IDT_P_DPL0_INTR_32 ((IDT_P << 7) + (IDT_DPL0 << 5) + IDT_INTR_32)
#define IDT_P_DPL3_INTR_32 ((IDT_P << 7) + (IDT_DPL3 << 5) + IDT_INTR_32)

#endif //__KERNEL_GLOBAL_H
