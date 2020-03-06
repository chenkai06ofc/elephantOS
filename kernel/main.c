#include "interrupt.h"
#include "debug.h"
#include "../lib/string.h"
#include "../device/timer.h"
#include "../device/console.h"
#include "../device/keyboard.h"
#include "../mm/memory.h"
#include "../thread/thread.h"

void test_thread_func(void* arg) {
    char* p = arg;
    int a = 0;
    while (1) {
        console_put_str(p);
    }
}

int main(void) {
    intr_disable();
    idt_init();
    mem_init();
    timer_init();
    thread_init();
    console_init();
    keyboard_init();

    thread_start("test1", 10, test_thread_func, "123456 ");
    thread_start("test2", 10, test_thread_func, "abcdef ");
    thread_start("test3", 10, test_thread_func, "...... ");

    intr_enable();
    int a = 0;

    while(1);
}