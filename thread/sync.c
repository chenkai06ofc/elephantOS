#include "sync.h"
#include "../kernel/interrupt.h"
#include "../kernel/debug.h"
#include "../lib/common.h"
#include "../lib/stdint.h"
#include "../lib/kernel/list.h"

void semaphore_init(struct semaphore* psem, uint8_t value) {
    psem->value = value;
    list_init(&psem->block_list);
}

void semaphore_down(struct semaphore* psem) {
    enum intr_status prev_status = intr_disable();
    while (psem->value == 0) {
        struct task_struct* current = current_thread();
        list_append(&psem->block_list, &current->status_list_tag);
        thread_block();
    }
    psem->value--;
    set_intr_status(prev_status);
}

void semaphore_up(struct semaphore* psem) {
    enum intr_status prev_status = intr_disable();
    struct list_node* node = list_pop(&psem->block_list);
    if (node != NULL) {
        struct task_struct* pthread = field_to_struct_ptr(struct task_struct, status_list_tag, node);
        thread_unblock(pthread);
    }
    psem->value++;
    set_intr_status(prev_status);
}

void lock_init(struct lock* plock) {
    plock->holder = NULL;
    plock->holder_entry_nr = 0;
    semaphore_init(&plock->sem, 1);
}

void lock_acquire(struct lock* plock) {
    struct task_struct* current = current_thread();
    if (plock->holder == current) {
        plock->holder_entry_nr++;
    } else {
        semaphore_down(&plock->sem);
        plock->holder = current;
        plock->holder_entry_nr = 1;
    }
}

void lock_release(struct lock* plock) {
    struct task_struct* current = current_thread();
    ASSERT(plock->holder == current);
    plock->holder_entry_nr--;
    if (plock->holder_entry_nr == 0) {
        plock->holder = NULL;
        semaphore_up(&plock->sem);
    }
}