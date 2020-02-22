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
    while(1);
}

