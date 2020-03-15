/**
 * Macros related to x86 architecture.
 */
#ifndef __KERNEL_X86_H
#define __KERNEL_X86_H

#define PG_SIZE             0x1000  // 4096

// eflags
#define EFLAGS_MBS          (1 << 1) // must be set
#define EFLAGS_IF1          (1 << 9) // enable interrupt
#define EFLAGS_IF0          (0 << 9) // disable interrupt
#define EFLAGS_IOPL0        (0 << 12)

// GDT
#define RPL0    0
#define RPL1    1
#define RPL2    2
#define RPL3    3

#define TI_GDT      (0 << 2)
#define TI_LDT      (1 << 2)

// pre-defined selectors
#define SELECTOR_K_CODE     ((1 << 3) + TI_GDT + RPL0)
#define SELECTOR_K_DATA     ((2 << 3) + TI_GDT + RPL0)
#define SELECTOR_K_STACK    SELECTOR_K_DATA
#define SELECTOR_TSS        ((4 << 3) + TI_GDT + RPL0)
#define SELECTOR_U_CODE     ((5 << 3) + TI_GDT + RPL3)
#define SELECTOR_U_DATA     ((6 << 3) + TI_GDT + RPL3)
#define SELECTOR_U_STACK    SELECTOR_U_DATA

// IDT
#define IDT_P           (1 << 7)
#define IDT_DPL0        (0 << 5)
#define IDT_DPL3        (3 << 5)
#define IDT_INTR_32     0xE
#define IDT_TRAP_32     0xF

#define IDT_P_DPL0_INTR_32      (IDT_P + IDT_DPL0 + IDT_INTR_32)
#define IDT_P_DPL3_INTR_32      (IDT_P + IDT_DPL3 + IDT_INTR_32)

#endif //__KERNEL_X86_H
