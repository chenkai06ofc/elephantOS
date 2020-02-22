#include "memory.h"
#include "stdint.h"
#include "bitmap.h"
#include "global.h"
#include "print.h"

#define PG_SIZE 0x1000 // 4096

#define MEM_BITMAP_BASE     0xc009a000
#define KERNEL_HEAP_START   0xc0100000

#define PDE_IDX(addr)   ((addr & 0xffc00000) >> 22)
#define PTE_IDX(addr)   ((addr & 0x003ff000) >> 12)

struct paddr_pool {
    struct bitmap bitmap;
    uint32_t paddr_start;
    uint32_t pool_size;
};

struct paddr_pool p_kernel_pool, p_user_pool;
struct vaddr_pool v_kernel_pool;

static void mem_pool_init();

void mem_init() {
    put_str("mem_init start\n");
    mem_pool_init();
    put_str("mem_init done\n");
}

static void mem_pool_init() {
    put_str("mem_pool_init start\n");
    uint32_t total_mem_bytes = *((uint32_t*)0x800);
    put_str("total mem of machine: ");
    put_int(total_mem_bytes);
    put_str("\n");

    uint32_t used_mem_bytes = 0x100000 + /* page table size */ PG_SIZE * 256;
    uint32_t free_mem_bytes = total_mem_bytes - used_mem_bytes;
    uint32_t free_page_cnt = free_mem_bytes / PG_SIZE;
    // bitmap length is in bytes
    uint32_t total_bitmap_len = free_page_cnt / 8;
    uint32_t kernel_bitmap_len = total_bitmap_len / 2;
    uint32_t user_bitmap_len = total_bitmap_len - kernel_bitmap_len;
    uint32_t kernel_page_cnt = kernel_bitmap_len * 8;
    uint32_t user_page_cnt = user_bitmap_len * 8;

    p_kernel_pool.paddr_start = used_mem_bytes;
    p_user_pool.paddr_start = used_mem_bytes + kernel_page_cnt * PG_SIZE;

    p_kernel_pool.pool_size = kernel_page_cnt * PG_SIZE;
    p_user_pool.pool_size = user_page_cnt * PG_SIZE;

    p_kernel_pool.bitmap.len_in_bytes = kernel_bitmap_len;
    p_user_pool.bitmap.len_in_bytes = user_bitmap_len;

    p_kernel_pool.bitmap.bits = (void*) MEM_BITMAP_BASE;
    p_user_pool.bitmap.bits = (void*) (MEM_BITMAP_BASE + kernel_bitmap_len);

    put_str("kernel_pool bitmap start: ");
    put_int((int)p_kernel_pool.bitmap.bits);
    put_str("\n");
    put_str("kernel_pool paddr start: ");
    put_int((int)p_kernel_pool.paddr_start);
    put_str("\n");

    put_str("user_pool bitmap start: ");
    put_int((int)p_user_pool.bitmap.bits);
    put_str("\n");
    put_str("user_pool paddr start: ");
    put_int((int)p_user_pool.paddr_start);
    put_str("\n");

    // clear bitmap
    bitmap_init(&p_kernel_pool.bitmap);
    bitmap_init(&p_user_pool.bitmap);
    
    /* init virtual kernel pool */
    v_kernel_pool.bitmap.len_in_bytes = kernel_bitmap_len;
    v_kernel_pool.bitmap.bits = (void*) (MEM_BITMAP_BASE + kernel_bitmap_len + user_bitmap_len);
    v_kernel_pool.vaddr_start = KERNEL_HEAP_START;
    bitmap_init(&v_kernel_pool.bitmap);

    put_str("mem_pool_init done\n");
}


/** allocate cnt virtual pages from virtual memory pool identified by pf */
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

/** get pte pointer of vaddr */
uint32_t* pte_ptr(uint32_t vaddr) {
    return (uint32_t*)(0xffc00000 | ((vaddr & 0xffc00000) >> 10) | ((vaddr & 0x003ff000) >> 10));
}

/** get pde pointer of vaddr */
uint32_t* pde_ptr(uint32_t vaddr) {
    return (uint32_t*)(0xfffff000 | (PDE_IDX(vaddr) * 4));
}

/** allocate 1 page from physical memory pool pool_ptr */
static void* palloc(struct paddr_pool* pool_ptr) {
    uint32_t start_idx = bitmap_scan(&pool_ptr->bitmap, 1);
    if (start_idx == -1) {
        return NULL;
    } else {
        bitmap_set(&pool_ptr->bitmap, start_idx, 1);
        return (void*)(pool_ptr->paddr_start + start_idx * PG_SIZE);
    }
}
