#ifndef __LIB_STDIO_H
#define __LIB_STDIO_H

#include "stdint.h"
#include "common.h"

uint32_t vsprintf(char* str, const char* format, va_list ap);
uint32_t sprintf(char* str, const char* format, ...);
uint32_t printf(const char* format, ...);
#endif //__LIB_STDIO_H
