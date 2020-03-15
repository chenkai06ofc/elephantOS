#ifndef __KERNEL_SYSCALL_H
#define __KERNEL_SYSCALL_H

#include "../lib/stdint.h"
#include "../thread/thread.h"
void syscall_init(void);

pid_t getpid();
uint32_t write(void* buf, uint32_t count);
#endif //__KERNEL_SYSCALL_H
