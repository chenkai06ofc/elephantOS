#include "print.h"
#include "interrupt.h"

int main(void) {
    put_str("hello world...\n");
    idt_init();
    asm volatile("sti");
    while(1);
}

