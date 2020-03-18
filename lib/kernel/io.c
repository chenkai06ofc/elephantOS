#include "io.h"
#include "../common.h"
#include "../stdio.h"
#include "../../device/console.h"

uint32_t printk(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buf[1024]; // hard code buffer
    uint32_t cnt = vsprintf(buf, format, args);
    return sys_write(buf, cnt);
}

uint32_t sys_write(char* buf, uint32_t count) {
    console_put_char_seq(buf, count);
    return count;
}