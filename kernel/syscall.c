#include "syscall.h"
#include "../lib/common.h"
#include "../thread/thread.h"
#include "../device/console.h"

#define _syscall0(syscall_no)   ({  \
    int retval;                 \
    asm volatile("int $0x80" : "=a" (retval) : "a" (syscall_no) : "memory"); \
    retval;                     \
})

#define _syscall1(syscall_no, arg1) ({  \
    int retval;                 \
    asm volatile("int $0x80"     \
        : "=a" (retval)         \
        : "a" (syscall_no), "b" (arg1)  \
        : "memory");            \
    retval;                     \
})

#define _syscall2(syscall_no, arg1, arg2) ({    \
    int retval;                 \
    asm volatile("int $0x80"     \
        : "=a" (retval)         \
        : "a" (syscall_no), "b" (arg1), "c" (arg2)  \
        : "memory");            \
    retval;                     \
})

#define _syscall3(syscall_no, arg1, arg2, arg3) ({  \
    int retval;                 \
    asm volatile("int $0x80"     \
        : "=a" (retval)         \
        : "a" (syscall_no), "b" (arg1), "c" (arg2), "d" (arg3)  \
        : "memory");            \
    retval;                     \
})

#define SYSCALL_CNT     100
#define NR_GETPID       1
#define NR_WRITE        2

func_addr syscall_handler_list[SYSCALL_CNT];

static uint32_t sys_write(void* buf, uint32_t count);

void syscall_init(void) {
    syscall_handler_list[NR_GETPID] = sys_getpid;
    syscall_handler_list[NR_WRITE] = sys_write;
}

pid_t getpid(void) {
    return _syscall0(NR_GETPID);
}

uint32_t write(char* buf, uint32_t count) {
    return _syscall2(NR_WRITE, buf, count);
}

/**  move this function to io.c */
static uint32_t sys_write(char* buf, uint32_t count) {
    console_put_char_seq(buf, count);
    return count;
}