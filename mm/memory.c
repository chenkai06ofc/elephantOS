#include "memory.h"
#include "bitmap.h"
#include "addr_pool.h"
#include "../kernel/x86.h"
#include "../thread/thread.h"
#include "../lib/stdint.h"
#include "../lib/common.h"
#include "../lib/string.h"
#include "../lib/kernel/print.h"

#define MEM_BITMAP_BASE     0xc009a000
#define KERNEL_HEAP_START   0xc0100000

struct paddr_pool p_kernel_pool, p_user_pool;
struct vaddr_pool v_kernel_pool;

/* kernel mem_block descriptors */
struct mem_block_desc k_mb_descs[MEM_BLOCK_SPEC_CNT];

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

    /* init kernel mem_block descriptors */
    uint32_t block_size = 16;
    for (int i = 0; i < MEM_BLOCK_SPEC_CNT; i++) {
        mem_block_desc_init(&k_mb_descs[i], block_size);
        block_size *= 2;
    }

    put_str("mem_init done\n");
}

/** allocate cnt virtual pages, and map to physical pages, return start vaddr if succeed, return NULL if failed */
static void* malloc_page(enum pool_flag pf, uint32_t cnt) {
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

static void dealloc_paage(enum pool_flag pf, void* vaddr, uint32_t cnt) {
    struct vaddr_pool* v_pool = (pf == PF_KERNEL) ? &v_kernel_pool : &current_thread()->vaddr_pool;
}

void* get_kernel_pages(uint32_t cnt) {
    void* vaddr = malloc_page(PF_KERNEL, cnt);
    if (vaddr != NULL) {
        memset(vaddr, 0, cnt * PG_SIZE);
    }
    return vaddr;
}

void alloc_user_page_at(void* vaddr) {
    uint32_t page_idx = ((uint32_t)vaddr) / PG_SIZE;
    valloc_page_at(&current_thread()->vaddr_pool, (uint32_t)vaddr);
    void* paddr = paddr_alloc(&p_user_pool);
    map_vaddr_paddr((uint32_t) vaddr, (uint32_t) paddr);
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

static void unmap_vaddr(uint32_t vaddr) {
    uint32_t* pte_ptr = PTE_PTR(vaddr);
    *pte_ptr &= PG_P_0;
    // update TLB
    asm volatile("invlpg %0" : : "m" (vaddr));
}

void mem_block_desc_init(struct mem_block_desc* desc_ptr, uint32_t block_size) {
    desc_ptr->block_size = block_size;
    desc_ptr->cnt_per_arena = (PG_SIZE - sizeof(struct arena)) / block_size;
    list_init(&desc_ptr->free_list_head);
}

/** return addr of ith mem_block in an arena */
static struct mem_block* ith_block(struct arena* a, uint32_t i) {
    return (struct mem_block*) ((uint32_t)a + sizeof(struct arena) + i * a->desc->block_size);
}

/** return containing arena of a mem_block */
static struct arena* block2arena(struct mem_block* b) {
    return (struct arena*) ((uint32_t)b & 0xfffff000);
}

void* sys_malloc(uint32_t size_in_bytes) {
    struct task_struct* current = current_thread();
    enum pool_flag flag = (current->pgdir == NULL) ? PF_KERNEL : PF_USER;

    struct arena* a;
    if (size_in_bytes > 1024) {
        uint32_t page_cnt = CEIL(size_in_bytes + sizeof(struct arena), PG_SIZE);
        a = malloc_page(flag, page_cnt);
        a->desc = NULL;
        a->free_cnt = page_cnt;
        a->large = true;
        return (void*) (a + 1);
    } else {
        struct mem_block_desc* desc = (current->pgdir == NULL) ? k_mb_descs : current->u_mb_descs;
        while (desc->block_size < size_in_bytes) {
            desc++;
        }

        if (list_empty(&desc->free_list_head)) {
            a = malloc_page(flag, 1);
            a->desc = desc;
            a->free_cnt = desc->cnt_per_arena;
            a->large = false;
            for (int i = 0; i < desc->cnt_per_arena; i++) {
                list_append(&desc->free_list_head, &ith_block(a, i)->hook);
            }
        }

        struct mem_block* b = (struct mem_block*) list_pop(&desc->free_list_head);
        block2arena(b)->free_cnt--;
        return (void*)b;
    }
}

void sys_free(void* ptr) {
    struct mem_block* b = (struct mem_block*) ptr;
    struct arena* a = block2arena(b);
    if (a->large) {
        struct task_struct* current = current_thread();
        struct paddr_pool* p_pool = (current->pgdir == NULL) ? &p_kernel_pool : &p_user_pool;
        struct vaddr_pool* v_pool = (current->pgdir == NULL) ? &v_kernel_pool : &current->vaddr_pool;
        uint32_t pg_cnt = a->free_cnt;
        uint32_t page_addr = (uint32_t) a;
        for (int i = 0; i < pg_cnt; i++) {
            uint32_t vaddr = page_addr + i * PG_SIZE;
            uint32_t paddr = PHY_ADDR(vaddr);
            paddr_dealloc(p_pool, (void*) paddr);
            unmap_vaddr(vaddr);
        }
        vaddr_dealloc(v_pool, (void*) page_addr, pg_cnt);
    } else {
        list_append(&a->desc->free_list_head, &b->hook);
        a->free_cnt++;
    }
}