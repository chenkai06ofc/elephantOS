#include "string.h"
#include "../lib/stdint.h"

/** set size bytes from start_ addr to value */
void memset(void* start_, uint8_t value, uint32_t size) {
    uint8_t* start = (uint8_t*)start_;
    int i = 0;
    for (i = 0; i < size; i++) {
        *(start + i) = value;
    }
}

/** copy size bytes from src_ addr to dst_ */
void memcpy(void* dst_, const void* src_, uint32_t size) {
    uint8_t* dst = (uint8_t*) dst_;
    const uint8_t* src = (uint8_t*) src_;
    int i;
    if (dst == src) {
        return;
    } else if (dst < src) {
        for (i = 0; i < size; i++) {
            *(dst + i) = *(src + i);
        }
    } else {
        for (i = size - 1; i >= 0; i--) {
            *(dst + i) = *(src + i);
        }
    }
}

/** compare size bytes start from a_ and b_, return 0: a_ = b_, 1: a_ > b_, -1: a_ < b_ */
int memcmp(const void* a_, const void* b_, uint32_t size) {
    const char* a = a_;
    const char* b = b_;
    char xa, xb;
    while (size > 0) {
        xa = *a;
        xb = *b;
        if (xa != xb) {
            return xa > xb ? 1 : -1;
        }
        a++;
        b++;
        size--;
    }
    return 0;
}

/** copy string from src_ to dst_ */
char* strcpy(char* dst_, const char* src_) {
    char* r = dst_;
    while (1) {
        if (!(*dst_ = *src_)) {
            break;
        }
        dst_++;
        src_++;
    }
    return r;
}
/** compare 2 strings, 1: a > b, 0: a == b, -1; a < b */
uint8_t strcmp(const char* a, const char* b) {
    while (*a != 0 & *a == *b) {
        a++;
        b++;
    }
    return (*a < *b) ? -1 : ((*a > *b) ? 1 : 0);
}

/** length of string */
uint32_t strlen(const char* str) {
    const char* p = str;
    while (*p) {
        p++;
    }
    return (p - str);
}
