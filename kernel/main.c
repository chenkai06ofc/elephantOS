#include "interrupt.h"
#include "memory.h"
#include "debug.h"
#include "../lib/kernel/print.h"
#include "../device/timer.h"
#include "../thread/thread.h"

void test_thread_func(void* arg) {
    char* p = arg;
    while (1) {
        put_str(p);put_str("\n");
    }
}

int main(void) {
    put_str("hello world...\n");
    timer_init();
    mem_init();
    idt_init();
    //asm volatile("sti");

    thread_start("test_name", 30, test_thread_func, "test msg");

    while(1);
}

