#include "memory.h"
#include "bitmap.h"
#include "addr_pool.h"
#include "../thread/thread.h"
#include "../lib/stdint.h"
#include "../lib/common.h"
#include "../lib/string.h"
#include "../lib/kernel/print.h"

#define PG_SIZE 0x1000 // 4096

#define MEM_BITMAP_BASE     0xc009a000
#define KERNEL_HEAP_START   0xc0100000

#define PDE_IDX(addr)   ((addr & 0xffc00000) >> 22)
#define PTE_IDX(addr)   ((addr & 0x003ff000) >> 12)
/* addr is uint32_t, return uint32_t* */
#define PDE_PTR(addr)   (uint32_t*)(0xfffff000 | ((addr & 0xffc00000) >> 20));
#define PTE_PTR(addr)   (uint32_t*)(0xffc00000 | ((vaddr & 0xffc00000) >> 10) | ((vaddr & 0x003ff000) >> 10));
/* get physical address from virtual address, vaddr is uint32_t, return uint32_t */
#define PHY_ADDR(vaddr) (((*PTE_PTR(vaddr)) & 0xfffff000) | (vaddr & 0x00000fff))

struct paddr_pool p_kernel_pool, p_user_pool;
struct vaddr_pool v_kernel_pool;

static void map_vaddr_paddr(uint32_t vaddr, uint32_t paddr);

void mem_init() {
    put_str("mem_init start\n");
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

    put_str("mem_init done\n");
}

/** allocate cnt virtual pages, and map to physical paegs, return start vaddr if succeed, return NULL if failed */
void* malloc_page(enum pool_flag pf, uint32_t cnt) {
    struct vaddr_pool* v_pool = (pf == PF_KERNEL) ? &v_kernel_pool : &current_thread()->vaddr_pool;
    void* vaddr = vaddr_alloc(v_pool, cnt);
    if (vaddr == NULL) {
        return NULL;
    }
    uint32_t vaddr_scalar = (uint32_t) vaddr;
    struct paddr_pool* p_pool = (pf == PF_KERNEL) ? &p_kernel_pool : &p_user_pool;
    for (int i = 0; i < cnt; i++) {
        void* p_addr = paddr_alloc(p_pool);
        map_vaddr_paddr(vaddr_scalar, (uint32_t) p_addr);
        vaddr_scalar += PG_SIZE;
    }
    return vaddr;
}

void* get_kernel_pages(uint32_t cnt) {
    void* vaddr = malloc_page(PF_KERNEL, cnt);
    if (vaddr != NULL) {
        memset(vaddr, 0, cnt * PG_SIZE);
    }
    return vaddr;
}

static void map_vaddr_paddr(uint32_t vaddr, uint32_t paddr) {
    uint32_t* pde_ptr = PDE_PTR(vaddr);
    uint32_t* pte_ptr = PTE_PTR(vaddr);
    // PDE not present
    if (!(*pde_ptr) & PG_P_1) {
        uint32_t pt_page_paddr = (uint32_t)paddr_alloc(&p_kernel_pool);
        *pde_ptr = pt_page_paddr | PG_US_U | PG_RW_W | PG_P_1;
        // pt_page_paddr is physical address, cannot be used directly
        memset((void*)((uint32_t)pte_ptr & 0xfffff000), 0, PG_SIZE);
    }
    *pte_ptr = paddr | PG_US_U | PG_RW_W | PG_P_1;
}