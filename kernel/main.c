#include "interrupt.h"
#include "syscall.h"
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
pid_t pid_a, pid_b;

int main(void) {
    intr_disable();
    idt_init();
    mem_init();
    timer_init();
    thread_init();
    console_init();
    keyboard_init();
    tss_init();
    syscall_init();

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
        console_put_str(" pid_a:");
        console_put_int(pid_a);
    }
}

static void k_thread_b(void* args) {
    while (1) {
        console_put_str(" pid_b:");
        console_put_int(pid_b);
    }
}

static void u_prog_a(void) {
    pid_a = getpid();
    while (1);
}

static void u_prog_b(void) {
    pid_b = getpid();
    while (1);
}