#include "interrupt.h"
#include "syscall.h"
#include "debug.h"
#include "../lib/string.h"
#include "../lib/stdio.h"
#include "../lib/kernel/io.h"
#include "../device/timer.h"
#include "../device/console.h"
#include "../device/keyboard.h"
#include "../device/ide.h"
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
static void k_thread_c(void* args);
static void u_prog_a(void);
static void u_prog_b(void);
static void u_prog_c(void);

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
    ide_init();
    filesys_init();

    char buf[100];
    sprintf(buf, "test %x for sprintf\n", 0xf4);
    console_put_str(buf);

//    thread_start("test1", 10, test_thread_func, "123456 ");
//    thread_start("test2", 10, test_thread_func, "abcdef ");
//    thread_start("test3", 10, test_thread_func, "...... ");
    thread_start("thread1", 10, k_thread_a, "argA");
    thread_start("thread2", 10, k_thread_b, "argB");
    thread_start("thread2", 10, k_thread_c, "argB");
//    process_execute("process1", u_prog_a);
//    process_execute("process2", u_prog_b);
//    process_execute("process3", u_prog_c);

    intr_enable();
    int a = 0;

    while(1);
}

static void k_thread_a(void* args) {
    printk("1 2 3 ");
    printk("4 5 6 ");
    sleep(400);
    printk("7 8 9 ");
    intr_disable();
    while (1);
}

static void k_thread_b(void* args) {
    while (1) {
        printk("BBBBBBBB ");
        printk("\b\b\b\b ");
    }
    //while (1);
}

static void k_thread_c(void* args) {
    while (1) {
        printk("CCCCCCCC ");
        printk("\b\b\b\b ");
    }
    //while (1);
}

static void u_prog_a(void) {
    printf("pid of prog_a is %d !\n", getpid());
    void* p = malloc(20);
    printf("20 bytes: 0x%x \n", (uint32_t)p);
    p = malloc(10);
    printf("10 bytes: 0x%x \n", (uint32_t)p);
    p = malloc(30);
    printf("30 bytes: 0x%x \n", (uint32_t)p);
    p = malloc(500);
    printf("500 bytes 1: 0x%x \n", (uint32_t)p);
    free(p);
    p = malloc(500);
    printf("500 bytes 2: 0x%x \n", (uint32_t)p);
    p = malloc(500);
    printf("500 bytes 3: 0x%x \n", (uint32_t)p);
    p = malloc(500);
    printf("500 bytes 4: 0x%x \n", (uint32_t)p);
    p = malloc(500);
    printf("500 bytes 5: 0x%x \n", (uint32_t)p);
    free(p);
    p = malloc(500);
    printf("500 bytes 6: 0x%x \n", (uint32_t)p);
    p = malloc(500);
    printf("500 bytes 7: 0x%x \n", (uint32_t)p);
    p = malloc(500);
    printf("500 bytes 8: 0x%x \n", (uint32_t)p);
    p = malloc(500);
    printf("500 bytes 9: 0x%x \n", (uint32_t)p);
    while (1);
}

static void u_prog_b(void) {
    printf("pid of prog_b is %d !\n", getpid());
    void* p = malloc(33);
    printf("allocated mem for prog b: 0x%x \n", (uint32_t)p);
    while (1);
}

static void u_prog_c(void) {
    printf("to print string %s inside\n", "hihi");
    while (1);
}