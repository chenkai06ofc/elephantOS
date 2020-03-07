#ifndef __KERNEL_MEMORY_H
#define __KERNEL_MEMORY_H
#include "bitmap.h"
#include "../lib/stdint.h"

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

enum pool_flag { PF_KERNEL, PF_USER };

void mem_init();
void* get_kernel_pages(uint32_t cnt);
/** allocate 1 user page at a specified virtual address */
void alloc_user_page_at(void* vaddr);
#endif //__KERNEL_MEMORY_H
