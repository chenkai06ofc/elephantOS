#ifndef __LIB_STRING_H
#define __LIB_STRING_H
#include "stdint.h"
void memset(void* start, uint8_t value, uint32_t size);
void memcpy(void* dst, const void* src, uint32_t size);
int memcmp(const void* a_, const void* b_, uint32_t size);

char* strcpy(char* dst_, const char* src_);
uint32_t strlen(const char* str);
#endif //__LIB_STRING_H
