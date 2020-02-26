#ifndef __LIB_KERNEL_LIST_H
#define __LIB_KERNEL_LIST_H
#include "../stdint.h"

struct list_node {
    struct list_node* prev;
    struct list_node* next;
};

void list_init(struct list_node* head);
uint32_t list_len(struct list_node* head);
int list_empty(struct list_node* head);

/** insert element node before base */
void list_insert_before(struct list_node* base, struct list_node* node);
/** prepend node at head of the list */
void list_prepend(struct list_node* head, struct list_node* node);
/** append node at tail of the list */
void list_append(struct list_node* head, struct list_node* node);

void list_remove(struct list_node* node);
struct list_node* list_pop(struct list_node* head);

#define list_entry(struct_type_name, field_name, field_ptr) \
    (struct_type_name*) ((uint32_t)field_ptr - (uint32_t)(&((struct_type_name*)0)->field_name))

#endif //__LIB_KERNEL_LIST_H
