#include "addr_pool.h"
#include "bitmap.h"
#include "../kernel/debug.h"
#include "../lib/stdint.h"
#include "../lib/common.h"

#define PG_SIZE 0x1000 // 4096

void* vaddr_alloc(struct vaddr_pool* v_pool, uint32_t cnt) {
    uint32_t start_idx = bitmap_scan(&v_pool->bitmap, cnt);
    if (start_idx == -1) {
        return NULL;
    } else {
        for (int i = 0; i < cnt; i++) {
            bitmap_set(&v_pool->bitmap, start_idx + i, 1);
        }
        return (void *) (v_pool->vaddr_start + start_idx * PG_SIZE);
    }
}

void* paddr_alloc(struct paddr_pool* p_pool) {
    uint32_t idx = bitmap_scan(&p_pool->bitmap, 1);
    if (idx == -1) {
        return NULL;
    } else {
        bitmap_set(&p_pool->bitmap, idx, 1);
        return (void*)(p_pool->paddr_start + idx * PG_SIZE);
    }
}

void valloc_page_at(struct vaddr_pool* v_pool, uint32_t vaddr) {
    uint32_t page_idx = (vaddr - v_pool->vaddr_start) / PG_SIZE;
    ASSERT(!bitmap_test(&v_pool->bitmap, page_idx));
    bitmap_set(&v_pool->bitmap, page_idx, 1);
}