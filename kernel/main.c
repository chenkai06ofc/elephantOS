#include "interrupt.h"
#include "memory.h"
#include "debug.h"
#include "../lib/string.h"
#include "../lib/kernel/print.h"
#include "../device/timer.h"
#include "../thread/thread.h"

void test_thread_func(void* arg) {
    char* p = arg;
    while (1) {
        intr_disable();
        put_str(p);
        intr_enable();
    }
}

static void print_thread_name(struct list_node* p) {
    struct task_struct* thread = list_entry(struct task_struct, status_list_tag, p);
    put_str(thread->name);put_str("  ");
}

int main(void) {
    put_str("hello world...\n");
    intr_disable();
    idt_init();
    mem_init();
    timer_init();
    thread_init();

    for (int i = 0; i < 1000; i++) {
        put_str(" hello world13... ");
        put_int(i);
    }

//    thread_start("test1", 20, test_thread_func, "123456 ");
//    thread_start("test2", 20, test_thread_func, "abcdef ");
//    thread_start("test3", 20, test_thread_func, "><)(~$ ");

    intr_enable();
    while(1);
}