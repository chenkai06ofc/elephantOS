#ifndef __KERNEL_INTERRUPT_H
#define __KERNEL_INTERRUPT_H
#include "../lib/stdint.h"
typedef void* intr_handler_addr;
enum intr_status { INTR_OFF, INTR_ON };

void idt_init(void);
void register_intr_handler(uint8_t vec_no, intr_handler_addr function);
/** get current interrupt status */
enum intr_status get_intr_status(void);
/** set interrupt status */
void set_intr_status(enum intr_status);
/** enable interrupt, return the pre intr status */
enum intr_status intr_enable(void);
/** disable interrupt, return the pre intr status */
enum intr_status intr_disable(void);

#endif //__KERNEL_INTERRUPT_H
