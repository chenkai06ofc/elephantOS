#include "sleep.h"
#include "../kernel/debug.h"
#include "../lib/common.h"
#include "../lib/kernel/list.h"
#include "../mm/memory.h"

#define SLEEP_LIST_1_LEN        256
#define SLEEP_LIST_2_LEN        64
#define LIST_LEN                (SLEEP_LIST_1_LEN * SLEEP_LIST_2_LEN)

static struct list_node sleep_list_1[SLEEP_LIST_1_LEN];
static struct list_node sleep_list_2[SLEEP_LIST_2_LEN];
static struct list_node ext_list;

static uint32_t idx;

void sleep_struct_init(void) {
    for (int i = 0; i < SLEEP_LIST_1_LEN; i++) {
        list_init(&sleep_list_1[i]);
    }
    for (int i = 0; i < SLEEP_LIST_2_LEN; i++) {
        list_init(&sleep_list_2[i]);
    }
    list_init(&ext_list);
    idx = 0;
}

void sleep_list_append(struct task_struct* task, uint32_t delay) {
    ASSERT(delay > 0);
    struct sleep_item* item = (struct sleep_item*) sys_malloc(sizeof(struct sleep_item));
    item->task = task;
    item->expires = idx + delay;
    list_init(&item->hook);

    if (delay < SLEEP_LIST_1_LEN) {
        uint32_t new_idx = (delay + idx) % SLEEP_LIST_1_LEN;
        list_append(&sleep_list_1[new_idx], &item->hook);
    } else if (delay < LIST_LEN) {
        uint32_t idx2 = ((delay + idx) % LIST_LEN) / SLEEP_LIST_1_LEN;
        list_append(&sleep_list_2[idx2], &item->hook);
    } else {
        list_append(&ext_list, &item->hook);
    }
}

void proceed_time_slice(void (*func)(struct task_struct*)) {
    if (idx == (LIST_LEN - 1)) {
        struct list_node* node = ext_list.next;
        while (node != &ext_list) {
            struct sleep_item* item = field_to_struct_ptr(struct sleep_item, hook, node);
            item->expires -= LIST_LEN;
            if (item->expires < LIST_LEN) {
                struct list_node* node2 = node->next;
                list_remove(node);
                list_append(&sleep_list_2[item->expires / SLEEP_LIST_1_LEN], node);
                node = node2;
            }
        }
    }
    if ((idx % SLEEP_LIST_1_LEN) == SLEEP_LIST_1_LEN - 1) {
        uint32_t move_idx = ((idx + 1) / SLEEP_LIST_1_LEN) % SLEEP_LIST_2_LEN;
        struct list_node* head = &sleep_list_2[move_idx];
        struct list_node* node = head ->next;
        while (node != head) {
            struct list_node* node2 = node->next;
            list_remove(node);
            struct sleep_item* item = field_to_struct_ptr(struct sleep_item, hook, node);
            list_append(&sleep_list_1[item->expires % SLEEP_LIST_1_LEN], node);
            node = node2;
        }
    }

    idx = (idx + 1) % LIST_LEN;
    struct list_node* list_to_free = &sleep_list_1[idx % SLEEP_LIST_1_LEN];
    while(!list_empty(list_to_free)) {
        struct sleep_item* item = field_to_struct_ptr(struct sleep_item, hook, list_pop(list_to_free));
        func(item->task);
        sys_free(item);
    }
}
