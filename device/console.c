#include "console.h"
#include "../thread/thread.h"
#include "../thread/sync.h"
#include "../lib/kernel/print.h"
static struct lock console_lock;

void console_init(void) {
    lock_init(&console_lock);
}

void console_put_str(char* str) {
    lock_acquire(&console_lock);
    put_str(str);
    lock_release(&console_lock);
}

void console_put_char(uint8_t char_ascii) {
    lock_acquire(&console_lock);
    put_char(char_ascii);
    lock_release(&console_lock);
}

void console_put_int(uint32_t n) {
    lock_acquire(&console_lock);
    put_int(n);
    lock_release(&console_lock);
}