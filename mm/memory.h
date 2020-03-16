#ifndef __KERNEL_MEMORY_H
#define __KERNEL_MEMORY_H
#include "bitmap.h"
#include "../lib/stdint.h"
#include "../lib/kernel/list.h"

#define PG_P_1      1
#define PG_P_0      0
#define PG_RW_R     0
#define PG_RW_W     2
#define PG_US_S     0
#define PG_US_U     4

#define PDE_IDX(addr)   ((addr & 0xffc00000) >> 22)
#define PTE_IDX(addr)   ((addr & 0x003ff000) >> 12)
/* vaddr is uint32_t, return uint32_t* */
#define PDE_PTR(vaddr)   (uint32_t*)(0xfffff000 | ((vaddr & 0xffc00000) >> 20))
#define PTE_PTR(vaddr)   (uint32_t*)(0xffc00000 | ((vaddr & 0xffc00000) >> 10) | ((vaddr & 0x003ff000) >> 10))
/* get physical address from virtual address, vaddr is uint32_t, return uint32_t */
#define PHY_ADDR(vaddr) (((*PTE_PTR(vaddr)) & 0xfffff000) | (vaddr & 0x00000fff))

/* count of different mem_block specifications */
#define MEM_BLOCK_SPEC_CNT  7

enum pool_flag { PF_KERNEL, PF_USER };

struct arena {
    struct mem_block_desc* desc;
    // if large = true, means page frame count,
    // else means free mem_block count
    uint32_t free_cnt;
    bool large;
};

struct mem_block {
    struct list_node hook;
};

/* mem_block descriptor */
struct mem_block_desc {
    // in bytes
    uint32_t block_size;
    // how many blocks a arena can hold
    uint32_t cnt_per_arena;
    struct list_node free_list_head;
};

void mem_init();
/** allocate cnt kernel pages, and return the virtual address */
void* get_kernel_pages(uint32_t cnt);
/** allocate 1 user page at a specified virtual address */
void alloc_user_page_at(void* vaddr);

void mem_block_desc_init(struct mem_block_desc* desc_ptr, uint32_t block_size);

void* sys_malloc(uint32_t size_in_bytes);

void sys_free(void* ptr);
#endif //__KERNEL_MEMORY_H
