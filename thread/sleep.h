#ifndef __THREAD_SLEEP_H
#define __THREAD_SLEEP_H

#include "../lib/stdint.h"
#include "../lib/kernel/list.h"
#include "thread.h"

struct sleep_item {
    struct list_node hook;
    uint32_t expires;
    struct task_struct* task;
};

void sleep_struct_init(void);
void append_delay(struct task_struct* task, uint32_t delay);
void proceed_idx(void (*func)(struct task_struct*));

#endif //__THREAD_SLEEP_H
