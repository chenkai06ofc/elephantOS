#ifndef __KERNEL_SYSCALL_H
#define __KERNEL_SYSCALL_H

#include "../lib/stdint.h"
#include "../thread/thread.h"
void syscall_init(void);

pid_t getpid();
#endif //__KERNEL_SYSCALL_H
