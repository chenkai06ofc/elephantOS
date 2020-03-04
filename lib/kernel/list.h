#ifndef __LIB_KERNEL_LIST_H
#define __LIB_KERNEL_LIST_H
#include "../stdint.h"
#include "../common.h"

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
/** find whether the element is included in the list **/
bool list_has_elem(struct list_node* head, struct list_node* target);
/** traverse the list and apply func to all the nodes */
void list_traverse(struct list_node* head, void (*func)(struct list_node*));

#endif //__LIB_KERNEL_LIST_H
