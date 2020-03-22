#include "sleep.h"
#include "../kernel/debug.h"
#include "../lib/common.h"
#include "../lib/kernel/list.h"
#include "../mm/memory.h"

#define LIST_LEN    4096

static struct list_node wait_list[LIST_LEN];
static struct list_node ext_list;

static uint32_t idx;

void sleep_struct_init(void) {
    for (int i = 0; i < LIST_LEN; i++) {
        list_init(&wait_list[i]);
    }
    list_init(&ext_list);
    idx = 0;
}

void append_delay(struct task_struct* task, uint32_t delay) {
    ASSERT(delay > 0);
    struct sleep_item* item = (struct sleep_item*) sys_malloc(sizeof(struct sleep_item));
    item->task = task;
    item->expires = idx + delay;
    list_init(&item->hook);
    if (delay < LIST_LEN) {
        uint32_t new_idx = (delay + idx) % LIST_LEN;
        list_append(&wait_list[new_idx], &item->hook);
    } else {
        list_append(&ext_list, &item->hook);
    }
}

void proceed_idx(void (*func)(struct task_struct*)) {
    if (idx == (LIST_LEN - 1)) {
        struct list_node* node = ext_list.next;
        while (node != &ext_list) {
            struct sleep_item* item = field_to_struct_ptr(struct sleep_item, hook, node);
            item->expires -= LIST_LEN;
            if (item->expires < LIST_LEN) {
                struct list_node* node2 = node->next;
                list_remove(node);
                list_append(&wait_list[item->expires], node);
                node = node2;
            }
        }
    }
    idx = (idx + 1) % LIST_LEN;
    struct list_node* list_to_free = &wait_list[idx];
    while(!list_empty(list_to_free)) {
        struct sleep_item* item = field_to_struct_ptr(struct sleep_item, hook, list_pop(list_to_free));
        func(item->task);
        sys_free(item);
    }
}
