#include "interrupt.h"
#include "stdint.h"
#include "global.h"
#include "print.h"
#include "io.h"

#define PIC_M_CTRL_PORT 0x20
#define PIC_M_DATA_PORT 0x21
#define PIC_S_CTRL_PORT 0xa0
#define PIC_S_DATA_PORT 0xa1

#define IDT_DESC_CNT 0x21

struct gate_desc {
    uint16_t    func_offset_low;
    uint16_t    selector;
    uint8_t     dcount;
    uint8_t     attribute;
    uint16_t    func_offset_high;
};

static void idt_desc_init(void);
static void pic_init(void);
static void intr_handler_init(void);
static void general_intr_handler(uint8_t vec_nr);

static struct gate_desc idt[IDT_DESC_CNT]; // IDT table

static char* intr_names[IDT_DESC_CNT];
intr_handler_addr intr_handler_list[IDT_DESC_CNT];

extern intr_handler_addr intr_entry_list[IDT_DESC_CNT];

void idt_init(void) {
    put_str("idt_init start\n");
    intr_handler_init();
    idt_desc_init();
    pic_init();

    uint64_t idt_setting = (((uint64_t)(uint32_t)idt) << 16) + sizeof(idt) -1;
    // this code will generate __stack_chk_fail_local symbol in interrupt.o
    asm volatile ("lidt %0" : : "m" (idt_setting));
}

static void intr_handler_init(void) {
    int i;
    for (i = 0; i < IDT_DESC_CNT; i++) {
        intr_names[i] = "unknown";
        intr_handler_list[i] = general_intr_handler;
    }
    intr_names[0] = "#DE Divide Error";
    intr_names[1] = "#DB Debug Exception";
    intr_names[2] = "NMI Interrupt";
    intr_names[3] = "#BP Breakpoint";
    intr_names[4] = "#OF Overflow";
    intr_names[5] = "#BR BOUND Range Exceeded";
    intr_names[6] = "#UD Invalid Opcode";
    intr_names[7] = "#NM Device Not Available";
    intr_names[8] = "#DF Double Fault";

    intr_names[10] = "#TS Invalid TSS";
    intr_names[11] = "#NP Segment Not Present";
    intr_names[12] = "#SS Stack-Segment Fault";
    intr_names[13] = "#TS General Protection";
    intr_names[14] = "#PF Page Fault";

    intr_names[16] = "#MF x87 FPU Floating-Point Error";
    intr_names[17] = "#AC Alignment Check";
    intr_names[18] = "#MC Machine Check";
    intr_names[19] = "#XM SIMD Floating-Point Exception";
    intr_names[20] = "#VE Virtualization Exception";
    intr_names[21] = "#CP Control Protection Exception";

    intr_names[0x20] = "Timer";
}

static void general_intr_handler(uint8_t vec_nr) {
    put_str("intr number: ");
    put_int((uint32_t)vec_nr);
    put_str("\n");
    put_str("intr name: ");
    put_str(intr_names[vec_nr]);
    put_str("\n");
}

static void make_idt_desc(struct gate_desc* p_desc, uint8_t attr, intr_handler_addr function) {
    p_desc->func_offset_low = ((uint32_t)function & 0x0000FFFF);
    p_desc->selector = SELECTOR_KERNEL_CODE;
    p_desc->dcount = 0;
    p_desc->attribute = attr;
    p_desc->func_offset_high = (((uint32_t)function & 0xFFFF0000) >> 16);
}

static void idt_desc_init(void) {
    int i;
    for (i = 0; i < IDT_DESC_CNT; i++) {
        make_idt_desc(&idt[i], IDT_P_DPL0_INTR_32, intr_entry_list[i]);
    }
    put_str("idt_desc_init done\n");
}


/** init 8259A */
static void pic_init(void) {
    /** init master */
    outb(PIC_M_CTRL_PORT, 0x11); // 0001_0001b
    outb(PIC_M_DATA_PORT, 0x20); // 0010_0000b
    outb(PIC_M_DATA_PORT, 0x04); // 0000_0100b
    outb(PIC_M_DATA_PORT, 0x01); // 0000_0001b

    /** init slave */
    outb(PIC_S_CTRL_PORT, 0x11); // 0001_0001b
    outb(PIC_S_DATA_PORT, 0x28); // 0010_1000b
    outb(PIC_S_DATA_PORT, 0x02); // 0000_0010b
    outb(PIC_S_DATA_PORT, 0x01); // 0000_0001b

    outb(PIC_M_DATA_PORT, 0xfe);
    outb(PIC_S_DATA_PORT, 0xff);

    put_str("pic_init done\n");
}