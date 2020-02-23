#ifndef __KERNEL_BITMAP_H
#define __KERNEL_BITMAP_H
#include "../lib/stdint.h"
struct bitmap {
    uint32_t len_in_bytes;
    uint8_t* bits;
};

/** Init bitmap */
void bitmap_init(struct bitmap* btmp);
/** Check whether bit_idx bit of bitmap is set */
int bitmap_test(struct bitmap* btmp, uint32_t bit_idx);
/** Apply consecutive cnt bits in bitmap, return start index if succeed, -1 if failed */
uint32_t bitmap_scan(struct bitmap* btmp, uint32_t cnt);
/** Set the bit_idx bit as value */
void bitmap_set(struct bitmap* btmp, uint32_t bit_idx, int8_t value);

#endif //__KERNEL_BITMAP_H
