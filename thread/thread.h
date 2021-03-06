#ifndef __THREAD_THREAD_H
#define __THREAD_THREAD_H

#include "../mm/addr_pool.h"
#include "../mm/memory.h"
#include "../lib/stdint.h"
#include "../lib/kernel/list.h"

typedef void (*thread_func)(void*);
typedef void (*process_func)(void);
typedef uint32_t pid_t;

enum task_status {
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_SLEEPING
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
    uint32_t fs;
    uint32_t gs;
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
    pid_t pid;
    enum task_status status;
    char name[16];
    uint8_t prio;
    uint8_t ticks;
    uint32_t elapsed_ticks;

    struct list_node status_list_tag;
    struct list_node all_list_tag;

    void* pgdir; // virtual address of thread's page directory
    struct vaddr_pool vaddr_pool;
    struct mem_block_desc u_mb_descs[MEM_BLOCK_SPEC_CNT]; // user mem_block descriptors
    uint32_t stack_magic;
};

void thread_init(void);
struct task_struct* thread_start(char* name, int prio, thread_func function, void* func_arg);
void process_execute(char* name, process_func func);
struct task_struct* current_thread();
/** switch current thread off the CPU to another thread **/
void schedule(void);
void thread_block(void);
void thread_unblock(struct task_struct* pthread);
/** relinquish CPU for a moment, but add itself to ready list */
void thread_yield(void);

void sleep(uint32_t ms);
/** thread related operations that be invoked in timer intr handler */
void thread_ops_in_timer(void);
/** An implementation of a system call. Get pid of current thread */
pid_t sys_getpid(void);
#endif //__THREAD_THREAD_H
