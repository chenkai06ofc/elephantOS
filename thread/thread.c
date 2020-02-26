#include "thread.h"
#include "../kernel/global.h"
#include "../kernel/memory.h"
#include "../kernel/interrupt.h"
#include "../lib/stdint.h"
#include "../lib/string.h"
#include "../lib/kernel/list.h"

#define PG_SIZE     0x1000

struct task_struct* main_thread;
struct list_node ready_list_head;
struct list_node all_list_head;

extern void switch_to(struct task_struct* current, struct task_struct* next);

struct task_struct* current_thread() {
    uint32_t esp;
    asm volatile ("movl %%esp, %0" : "=g" (esp));
    return (struct task_struct*) (esp & 0xfffff000);
}

static void kernel_thread(thread_func function, void* func_arg) {
    intr_enable();
    function(func_arg);
}

static void init_thread(struct task_struct* thread, char* name, int prio) {
    memset(thread, 0, sizeof(struct task_struct));
    strcpy(thread->name, name);
    if (thread == main_thread) {
        thread->status = TASK_RUNNING;
    } else {
        thread->status = TASK_READY;
    }

    thread->prio = prio;
    thread->ticks = prio;
    thread->elapsed_ticks = 0;
    thread->pgdir = NULL;
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

    list_append(&all_list_head, &thread->all_list_tag);
    list_append(&ready_list_head, &thread->status_list_tag);
    return thread;
}

static void make_main_thread(void) {
    main_thread = current_thread();
    init_thread(main_thread, "main", 31);
    list_append(&all_list_head, &main_thread->all_list_tag);
}

void schedule(void) {
    struct task_struct* current = current_thread();
    current->status = TASK_READY;
    current->ticks = current->prio;
    list_append(&ready_list_head, &current->status_list_tag);
    struct task_struct* next = list_entry(struct task_struct, status_list_tag, list_pop(&ready_list_head));
    next->status = TASK_RUNNING;
    switch_to(current, next);
}