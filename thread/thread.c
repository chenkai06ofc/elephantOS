#include "thread.h"
#include "../kernel/global.h"
#include "../kernel/memory.h"
#include "../kernel/interrupt.h"
#include "../kernel/debug.h"
#include "../lib/stdint.h"
#include "../lib/string.h"
#include "../lib/kernel/print.h"
#include "../lib/kernel/list.h"

#define PG_SIZE     0x1000

struct task_struct* main_thread;
struct list_node ready_list_head;
struct list_node all_list_head;

extern void switch_to(struct task_struct* current, struct task_struct* next);

static void make_main_thread(void);

void thread_init(void) {
    put_str("thread_init start\n");
    list_init(&ready_list_head);
    list_init(&all_list_head);
    make_main_thread();
    put_str("thread_init done\n");
}

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
    // current thread tag should not already exist in ready list
    ASSERT(!list_has_elem(&ready_list_head, &current->status_list_tag));
    if (current->status == TASK_RUNNING) {
        current->status = TASK_READY;
        current->ticks = current->prio;
        list_append(&ready_list_head, &current->status_list_tag);
    }

    struct task_struct* next = list_entry(struct task_struct, status_list_tag, list_pop(&ready_list_head));
    next->status = TASK_RUNNING;
    //put_str("switch to: ");put_str(next->name); put_str("\n");
    switch_to(current, next);
}

void thread_block(void) {
    enum intr_status prev_status = intr_disable();
    struct task_struct* current = current_thread();
    current->status = TASK_BLOCKED;
    schedule();
    set_intr_status(prev_status);
}

void thread_unblock(struct task_struct* pthread) {
    enum intr_status prev_status = intr_disable();
    if (pthread->status == TASK_BLOCKED) {
        pthread->status = TASK_READY;
        list_append(&ready_list_head, &pthread->status_list_tag);
    }
    set_intr_status(prev_status);
}
