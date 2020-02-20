#include "string.h"
#include "stdint.h"

/** set size bytes from start addr to value */
void memset1(void* start, uint8_t value, uint32_t size) {
    uint8_t* start_ = (uint8_t*)start;
    int i = 0;
    for (i = 0; i < size; i++) {
        *(start_ + i) = value;
    }
}

/** copy size bytes from src addr to dst */
void memcpy1(void* dst, const void* src, uint32_t size) {
    uint8_t* dst_ = (uint8_t*) dst;
    const uint8_t* src_ = (uint8_t*) src;
    int i;
    if (dst_ == src_) {
        return;
    } else if (dst_ < src_) {
        for (i = 0; i < size; i++) {
            *(dst_ + i) = *(src_ + i);
        }
    } else {
        for (i = size - 1; i >= 0; i--) {
            *(dst_ + i) = *(src_ + i);
        }
    }
}

