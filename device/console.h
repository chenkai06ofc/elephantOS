#ifndef __DEVICE_CONSOLE_H
#define __DEVICE_CONSOLE_H
#include "../lib/stdint.h"

void console_init(void);
void console_put_str(char* str);
void console_put_char(uint8_t char_ascii);
void console_put_int(uint32_t n);
/** Print a char sequence started from ch with cnt characters */
void console_put_char_seq(char* ch, uint32_t cnt);

#endif //__DEVICE_CONSOLE_H
