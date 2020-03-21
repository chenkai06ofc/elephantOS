#include "stdio.h"
#include "stdint.h"
#include "common.h"
#include "../kernel/syscall.h"
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
    char* format_ptr = (char*) format;
    char ch;
    uint32_t i;
    char* arg_str;
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
                    while (*str_ptr != 0) {
                        str_ptr++;
                    }
                    break;
                case 'd':
                    i = va_arg(ap, uint32_t);
                    itoa(i, str_ptr, 10);
                    while (*str_ptr != 0) {
                        str_ptr++;
                    }
                    break;
                case 's':
                    arg_str = va_arg(ap, char*);
                    strcpy(str_ptr, arg_str);
                    str_ptr += strlen(arg_str);
                    break;
            }
        }
        format_ptr++;
    }
    *str_ptr = 0;
    return strlen(str);
}

uint32_t sprintf(char* str, const char* format, ...) {
    va_list args;
    va_start(args, format);
    return vsprintf(str, format, args);
}

uint32_t printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buf[1024]; // hard code buffer
    uint32_t cnt = vsprintf(buf, format, args);
    return write(buf, cnt);
}
