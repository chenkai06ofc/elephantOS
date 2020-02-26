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
    idt_init();
    mem_init();
    timer_init();
    thread_init()
    intr_enable();

//    thread_start("test1", 30, test_thread_func, "aaaaaaaa");
//    thread_start("test2", 30, test_thread_func, "11111111");
//    thread_start("test3", 30, test_thread_func, "bbbbbbbb");

    while(1);
}