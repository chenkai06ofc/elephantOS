#ifndef __THREAD_THREAD_H
#define __THREAD_THREAD_H
#include "../lib/stdint.h"

typedef void (*thread_func)(void*);

struct task_struct* thread_start(char* name, int prio, thread_func function, void* func_arg);

enum task_status {
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED
};

struct intr_stack {
    uint32_t vec_no;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp_reg;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;
    // values below will be saved to stack automatically when interrupt happens
    uint32_t err_code;
    void (*eip) (void);
    uint32_t cs;
    uint32_t eflags;
    void* esp;
    uint32_t ss;
};

struct thread_stack {
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edi;
    uint32_t esi;
    void (*eip) (thread_func func, void* func_arg);
    // ret addr is just a placeholder to use ret to call function
    void (*unused_retaddr);
    thread_func function;
    void* func_arg;
};


struct task_struct {
    uint32_t* kstack;
    enum task_status status;
    uint8_t prio;
    char name[16];
    uint32_t stack_magic;
};





#endif //__THREAD_THREAD_H
