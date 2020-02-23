#include "interrupt.h"
#include "memory.h"
#include "../lib/kernel/print.h"
#include "../device/timer.h"

int main(void) {
    put_str("hello world...\n");
    timer_init();
    mem_init();
    idt_init();
    //asm volatile("sti");

    void* addr = get_kernel_pages(3);
    put_str("mem allocated: ");
    put_int((uint32_t)addr);
    put_str("\n");

    while(1);
}

