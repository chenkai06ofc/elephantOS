#ifndef __KERNEL_INTERRUPT_H
#define __KERNEL_INTERRUPT_H
typedef void* intr_handler_addr;

void idt_init(void);

enum intr_status { INTR_OFF, INTR_ON };

enum intr_status get_intr_status(void);
/** enable interrupt, return the pre intr status */
enum intr_status intr_enable(void);
/** disable interrupt, return the pre intr status */
enum intr_status intr_disable(void);

#endif //__KERNEL_INTERRUPT_H
