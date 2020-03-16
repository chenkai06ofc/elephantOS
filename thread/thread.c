#include "thread.h"
#include "sync.h"
#include "../kernel/x86.h"
#include "../kernel/interrupt.h"
#include "../kernel/debug.h"
#include "../mm/memory.h"
#include "../userprog/tss.h"
#include "../lib/common.h"
#include "../lib/stdint.h"
#include "../lib/string.h"
#include "../lib/kernel/print.h"
#include "../lib/kernel/list.h"

#define USER_PROG_VADDR_START       0x08048000
#define USER_PROG_VADDR_END         0xc0000000
#define USER_PROG_PAGE_CNT          ((USER_PROG_VADDR_END - USER_PROG_VADDR_START) / PG_SIZE)
#define USER_PROG_BITMAP_LEN        CEIL(USER_PROG_PAGE_CNT, 8)
#define USER_PROG_BITMAP_LEN_BY_PAGE    CEIL(USER_PROG_BITMAP_LEN, PG_SIZE)

#define USER_STACK3_VADDR           (0xc0000000 - PG_SIZE)


struct task_struct* main_thread;
struct list_node ready_list_head;
struct list_node all_list_head;

static struct lock pid_lock;
static pid_t next_pid = 0;

extern void switch_to(struct task_struct* current, struct task_struct* next);

static void make_main_thread(void);

void thread_init(void) {
    put_str("thread_init start\n");
    list_init(&ready_list_head);
    list_init(&all_list_head);
    lock_init(&pid_lock);
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

static void start_process(void* func) {
    enum intr_status prev_status = intr_disable();
    struct task_struct* thread = current_thread();
    thread->kstack = (uint32_t*)((uint32_t)thread + PG_SIZE - sizeof(struct intr_stack));
    struct intr_stack* i_stack = (struct intr_stack*)thread->kstack;
    memset(i_stack, 0, sizeof(struct intr_stack));
    i_stack->cs = SELECTOR_U_CODE;
    i_stack->ss = SELECTOR_U_STACK;
    i_stack->ds = i_stack->es = i_stack->fs = SELECTOR_U_DATA;
    alloc_user_page_at((void*)USER_STACK3_VADDR);
    i_stack->esp = (void*)(USER_STACK3_VADDR + PG_SIZE);
    i_stack->eip = func;
    i_stack->eflags = (EFLAGS_IOPL0 | EFLAGS_MBS | EFLAGS_IF1);
    // not necessary to enable interrupt, after iret, interrupt will be enabled because IF=1
    asm volatile ("movl %0, %%esp; jmp intr_exit" : : "g" (i_stack));
}

/** init a page of memory to a task_struct structure */
static void init_thread(struct task_struct* thread, char* name, int prio) {
    memset(thread, 0, sizeof(struct task_struct));

    lock_acquire(&pid_lock);
    thread->pid = next_pid++;
    lock_release(&pid_lock);

    strcpy(thread->name, name);
    thread->status = (thread == main_thread) ? TASK_RUNNING : TASK_READY;
    thread->prio = prio;
    thread->ticks = prio;
    thread->elapsed_ticks = 0;
    thread->pgdir = NULL;
    thread->kstack = (uint32_t*)((uint32_t)thread + PG_SIZE);
    thread->stack_magic = 0xFEFE8964;
}

/** fill thread's kernel stack with execute environment */
static void thread_create(struct task_struct* thread, thread_func function, void* func_arg) {
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

static void* create_page_dir(void) {
    void* pde_vaddr = get_kernel_pages(1);
    memcpy((void*)((uint32_t )pde_vaddr + 0x300*4), (void*)(0xfffff000 + 0x300*4), 1024);
    ((uint32_t*)pde_vaddr)[1023] = PHY_ADDR((uint32_t)pde_vaddr) | PG_US_U | PG_RW_W | PG_P_1;
    return pde_vaddr;
}

static void init_user_vaddr_pool(struct vaddr_pool* v_pool) {
    v_pool->vaddr_start = USER_PROG_VADDR_START;
    v_pool->bitmap.bits = get_kernel_pages(USER_PROG_BITMAP_LEN_BY_PAGE);
    v_pool->bitmap.len_in_bytes = USER_PROG_BITMAP_LEN;
    bitmap_init(&v_pool->bitmap);
}

void process_execute(char* name, process_func func) {
    struct task_struct* thread = get_kernel_pages(1);
    init_thread(thread, name, 10);
    thread_create(thread, start_process, func);
    thread->pgdir = create_page_dir();
    /* init user mem_block descriptors */
    uint32_t block_size = 16;
    for (int i = 0; i < MEM_BLOCK_SPEC_CNT; i++) {
        mem_block_desc_init(&thread->u_mb_descs[i], block_size);
        block_size *= 2;
    }
    init_user_vaddr_pool(&thread->vaddr_pool);

    list_append(&all_list_head, &thread->all_list_tag);
    list_append(&ready_list_head, &thread->status_list_tag);
}

static void make_main_thread(void) {
    main_thread = current_thread();
    init_thread(main_thread, "main", 31);
    list_append(&all_list_head, &main_thread->all_list_tag);
}

void schedule(void) {
    ASSERT(get_intr_status() == INTR_OFF);
    struct task_struct* current = current_thread();
    // current thread tag should not already exist in ready list
    ASSERT(!list_has_elem(&ready_list_head, &current->status_list_tag));
    if (current->status == TASK_RUNNING) {
        current->status = TASK_READY;
        current->ticks = current->prio;
        list_append(&ready_list_head, &current->status_list_tag);
    }
    struct task_struct* next = field_to_struct_ptr(struct task_struct, status_list_tag, list_pop(&ready_list_head));
    next->status = TASK_RUNNING;
    // update tss
    if (next->pgdir != NULL) {
        update_tss_esp(next);
    }
    // update page table
    uint32_t pde_paddr = (next->pgdir == NULL) ? 0x100000 : PHY_ADDR((uint32_t)next->pgdir);
    asm volatile ("movl %0, %%cr3" : : "r"(pde_paddr));
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

pid_t sys_getpid(void) {
    return current_thread()->pid;
}