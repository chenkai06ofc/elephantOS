#include "memory.h"
#include "stdint.h"
#include "bitmap.h"
#include "global.h"

#define PG_SIZE 0x1000 // 4096

#define PDE_IDX(addr)   ((addr & 0xffc00000) >> 22)
#define PTE_IDX(addr)   ((addr & 0x003ff000) >> 12)

struct paddr_pool {
    struct bitmap bitmap;
    uint32_t paddr_start;
    uint32_t pool_size;
};

struct paddr_pool p_kernel_pool, p_user_pool;
struct vaddr_pool v_kernel_pool;

static void* vaddr_get(enum pool_flag pf, uint32_t cnt) {
    if (pf == PF_KERNEL) {
        uint32_t start_idx = bitmap_scan(&v_kernel_pool.bitmap, cnt);
        if (start_idx == -1) {
            return NULL;
        } else {
            for (int i = 0; i < cnt; i++) {
                bitmap_set(&v_kernel_pool.bitmap, start_idx + i, 1);
            }
            return (void*)(v_kernel_pool.vaddr_start + start_idx * PG_SIZE);
        }
    } else {
        // v_user_pool not yet implemented
        return NULL;
    }
}

