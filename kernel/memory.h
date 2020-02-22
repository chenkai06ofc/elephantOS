#ifndef __KERNEL_MEMORY_H
#define __KERNEL_MEMORY_H
#include "stdint.h"
#include "bitmap.h"

#define PG_P_1      1
#define PG_P_0      0
#define PG_RW_R     0
#define PG_RW_W     2
#define PG_US_S     0
#define PG_US_U     4

enum pool_flag { PF_KERNEL, PF_USER };

/** virtual address pool */
struct vaddr_pool {
    struct bitmap bitmap;
    uint32_t vaddr_start;
};

void mem_init();
#endif //__KERNEL_MEMORY_H
