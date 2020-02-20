#include "print.h"
#include "interrupt.h"
#include "timer.h"

int main(void) {
    put_str("hello world...\n");
    timer_init();
    idt_init();
    asm volatile("sti");
    while(1);
}

