#ifndef __KERNEL_INTERRUPT_H
#define __KERNEL_INTERRUPT_H
typedef void* intr_handler_addr;

void idt_init(void);

#endif //__KERNEL_INTERRUPT_H
