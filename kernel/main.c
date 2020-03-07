#include "interrupt.h"
#include "debug.h"
#include "../lib/string.h"
#include "../device/timer.h"
#include "../device/console.h"
#include "../device/keyboard.h"
#include "../mm/memory.h"
#include "../thread/thread.h"
#include "../userprog/tss.h"

void test_thread_func(void* arg) {
    char* p = arg;
    int a = 0;
    while (1) {
        console_put_str(p);
    }
}

static void k_thread_a(void* args);
static void k_thread_b(void* args);
static void u_prog_a(void);
static void u_prog_b(void);
int test_a = 0, test_b = 0;

int main(void) {
    intr_disable();
    idt_init();
    mem_init();
    timer_init();
    thread_init();
    console_init();
    keyboard_init();
    tss_init();

//    thread_start("test1", 10, test_thread_func, "123456 ");
//    thread_start("test2", 10, test_thread_func, "abcdef ");
//    thread_start("test3", 10, test_thread_func, "...... ");
    thread_start("thread1", 10, k_thread_a, "argA");
    thread_start("thread2", 10, k_thread_b, "argB");
    process_execute("process1", u_prog_a);
    process_execute("process2", u_prog_b);

    intr_enable();
    int a = 0;

    while(1);
}

static void k_thread_a(void* args) {
    while (1) {
        console_put_str(" a:");
        console_put_int(test_a);
    }
}

static void k_thread_b(void* args) {
    while (1) {
        console_put_str(" b:");
        console_put_int(test_b);
    }
}

static void u_prog_a(void) {
    while(1) {
        test_a++;
    }
}

static void u_prog_b(void) {
    while(1) {
        test_b+=2;
    }
}