#include "thread.h"
#include "../kernel/memory.h"
#include "../lib/stdint.h"
#include "../lib/string.h"

#define PG_SIZE     0x1000

static void kernel_thread(thread_func* function, void* func_arg) {
    function(func_arg);
}

static void init_thread(struct task_struct* thread, char* name, int prio) {
    memset(thread, 0, sizeof(struct task_struct));
    strcpy(thread->name, name);
    thread->status = TASK_RUNNING;
    thread->prio = prio;
    thread->kstack = (uint32_t*)((uint32_t)thread + PG_SIZE);
    thread->stack_magic = 0xFEFE8964;
}

static void thread_create(struct task_struct* thread, thread_func function, void* func_arg) {
    thread->kstack -= sizeof(struct intr_stack);
    thread->kstack -= sizeof(struct thread_stack);

    struct thread_stack* t_stack = (struct thread_stack*)thread->kstack;
    memset(t_stack, 0, sizeof(struct thread_stack));
    t_stack->eip = kernel_thread;
    t_stack->function = function;
    t_stack->func_arg = func_arg;
}

struct task_struct* thread_start(char* name, int prio, thread_func function, void* func_arg) {
    struct task_struct* thread = get_kernel_pages(1);
    init_thread(thread, name, prio);
    thread_create(thread, function, func_arg);
    asm volatile ("movl %0, %%esp; pop %%ebp; pop %%ebx; \
        pop %%edi; pop %%esi; ret" : : "g"(thread->kstack) : "memory");
    return thread;
}