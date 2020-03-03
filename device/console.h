#ifndef __DEVICE_CONSOLE_H
#define __DEVICE_CONSOLE_H
#include "../lib/stdint.h"

void console_init(void);
void console_put_str(char* str);
void console_put_char(uint8_t char_ascii);
void console_put_int(uint32_t n);

#endif //__DEVICE_CONSOLE_H
