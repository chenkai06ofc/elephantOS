#include "../kernel/global.h"
#include "../lib/stdint.h"
#include "../lib/string.h"
#include "../lib/kernel/print.h"
#include "../thread/thread.h"

struct gdt_desc {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t attr_low;
    uint8_t limit_high_attr_high;
    uint8_t base_high;
};

struct tss {
    uint16_t backlink;
    uint16_t unused0;
    void* esp0;
    uint16_t ss0;
    uint16_t unused1[46];
    uint16_t io_base;
};

static struct tss tss;

static void update_desc(struct gdt_desc* desc_ptr, uint32_t base, uint32_t limit);

void tss_init() {
    put_str("tss_init start\n");
    uint32_t tss_size = sizeof(struct tss);
    memset(&tss, 0, tss_size);
    tss.ss0 = SELECTOR_K_STACK;
    tss.io_base = tss_size;
    update_desc((struct gdt_desc*)0xc0000922, (uint32_t)&tss, tss_size - 1);
    asm volatile("ltr %w0" : : "r" (SELECTOR_TSS));
    put_str("tss_init done\n");
}

void update_tss_esp(struct task_struct* pthread) {
    tss.esp0 = (void*)((uint32_t)pthread + PG_SIZE);
}

/** update base & limit of a descriptor */
static void update_desc(struct gdt_desc* desc_ptr, uint32_t base, uint32_t limit) {
    desc_ptr->base_low = base & 0x0000ffff;
    desc_ptr->base_mid = ((base & 0x00ff0000) >> 16);
    desc_ptr->base_high = base >> 24;
    desc_ptr->limit_low = limit & 0x0000ffff;
    desc_ptr->limit_high_attr_high = ((limit & 0x000f0000) >> 16) + (desc_ptr->limit_high_attr_high & 0xf0);
}