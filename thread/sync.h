#ifndef __THREAD_SYNC_H
#define __THREAD_SYNC_H

#include "thread.h"
#include "../lib/stdint.h"
#include "../lib/kernel/list.h"

struct semaphore {
    uint8_t value;
    struct list_node block_list;
};

struct lock {
    struct semaphore sem;
    struct task_struct* holder;
    uint32_t holder_entry_nr; // for re-entry
};

void semaphore_init(struct semaphore* psem, uint8_t value);
void semaphore_down(struct semaphore* psem);
void semaphore_up(struct semaphore* psem);
void lock_init(struct lock* plock);
void lock_acquire(struct lock* plock);
void lock_release(struct lock* plock);

#endif //__THREAD_SYNC_H
