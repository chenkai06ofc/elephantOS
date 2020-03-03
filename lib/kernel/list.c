#include "list.h"
#include "../stdint.h"
#include "../stdtypes.h"
#include "../../kernel/debug.h"

void list_init(struct list_node* head) {
    head->next = head;
    head->prev = head;
}

uint32_t list_len(struct list_node* head) {
    uint32_t len = 0;
    struct list_node* node = head->next;
    while (node != head) {
        len++;
        node = node->next;
    }
    return len;
}

int list_empty(struct list_node* head) {
    return head == head->next;
}

void list_insert_before(struct list_node* base, struct list_node* node) {
    base->prev->next = node;
    node->prev = base->prev;
    node->next = base;
    base->prev = node;
}

void list_prepend(struct list_node* head, struct list_node* node) {
    list_insert_before(head->next, node);
}

void list_append(struct list_node* head, struct list_node* node) {
    list_insert_before(head, node);
}

void list_remove(struct list_node* node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
}

struct list_node* list_pop(struct list_node* head) {
    if (list_empty(head)) {
        return NULL;
    }
    struct list_node* node = head->next;
    list_remove(node);
    return node;
}

bool list_has_elem(struct list_node* head, struct list_node* target) {
    struct list_node* node = head->next;
    while (node != head) {
        if (node == target) {
            return true;
        }
        node = node->next;
    }
    return false;
}

void list_traverse(struct list_node* head, void (*func)(struct list_node*)) {
    struct list_node* cur = head->next;
    while(cur != head) {
        func(cur);
        cur = cur->next;
    }
}
