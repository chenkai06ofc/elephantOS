#ifndef __KERNEL_SYSCALL_H
#define __KERNEL_SYSCALL_H

#include "../lib/stdint.h"
#include "../thread/thread.h"
void syscall_init(void);

/** system calls */
pid_t getpid();
uint32_t write(char* buf, uint32_t count);
void* malloc(uint32_t size_in_bytes);
void free(void* ptr);
#endif //__KERNEL_SYSCALL_H
