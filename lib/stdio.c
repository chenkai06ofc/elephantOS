#include "stdio.h"
#include "stdint.h"
#include "common.h"
#include "../lib/string.h"

/** integer to ascii */
static void itoa(uint32_t v, char* buf, uint8_t base) {
    if (v == 0) {
        buf[0] = '0';
        buf[1] = 0;
        return;
    } else {
        int len, idx = 0;
        while (v > 0) {
            uint32_t r = v % base;
            v = v / base;
            buf[idx] = (r < 10) ? (r + '0') : (r - 10 + 'A');
            idx++;
        }
        len = idx;
        idx = 0;
        while ((len - idx - 1) > idx) {
            // swap
            char t = buf[idx];
            buf[idx] = buf[len - idx - 1];
            buf[len - idx - 1] = t;
            idx++;
        }
        buf[len] = 0;
    }
}

/** return length of str */
uint32_t vsprintf(char* str, const char* format, va_list ap) {
    char* str_ptr = str;
    char* format_ptr = format;
    char ch;
    uint32_t i;
    while ((ch = *format_ptr) != 0) {
        if (ch != '%') {
            *(str_ptr++) = ch;
        } else {
            ch = *(++format_ptr);
            switch (ch) {
                // x is Hexadecimal
                case 'x':
                    i = va_arg(ap, uint32_t);
                    itoa(i, str_ptr, 16);
                    while (*str_ptr++ != 0);
                    break;
                case 'd':
                    i = va_arg(ap, uint32_t);
                    itoa(i, str_ptr, 10);
                    while (*str_ptr++ != 0);
                    break;
            }
        }
        format_ptr++;
    }
    *str_ptr = 0;
    return strlen(str);
}

uint32_t printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buf[1024]; // hard code buffer
    vsprintf(buf, format, args);
    return write(buf);
}
