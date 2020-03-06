#ifndef __MM_ADDR_POOL_H
#define __MM_ADDR_POOL_H

#include "bitmap.h"
#include "../lib/stdint.h"

/** virtual address pool */
struct vaddr_pool {
    struct bitmap bitmap;
    uint32_t vaddr_start;
};

/** physical address pool */
struct paddr_pool {
    struct bitmap bitmap;
    uint32_t paddr_start;
    uint32_t pool_size;
};

/** allocate cnt pages from virtual address pool.
 *  Note: only addresses are allocated, page table is not yet changed.
 **/
void* vaddr_alloc(struct vaddr_pool* v_pool, uint32_t cnt);

/** allocate 1 page from physical address pool
 * Note: only addresses are allocated, page table is not yet changed.
 **/
void* paddr_alloc(struct paddr_pool* p_pool);

#endif //__MM_ADDR_POOL_H
