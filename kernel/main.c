#include "print.h"
#include "interrupt.h"
#include "timer.h"
#include "memory.h"

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

