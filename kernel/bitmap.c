#include "bitmap.h"
#include "stdint.h"
#include "string.h"

void bitmap_init(struct bitmap* btmp) {
    memset(btmp->bits, 0, btmp->len_in_bytes);
}

int bitmap_test(struct bitmap* btmp, uint32_t bit_idx) {
    uint32_t byte_idx = bit_idx / 8;
    uint32_t bit_idx_in_byte = bit_idx % 8;
    return btmp->bits[byte_idx] & (1 << bit_idx_in_byte);
}

int bitmap_scan(struct bitmap* btmp, uint32_t cnt) {
    int start_zero_idx = -1;
    int cur_idx = 0;

    while(cur_idx / 8 < btmp ->len_in_bytes) {
        int8_t cur_byte = btmp->bits[cur_idx / 8];
        if (cur_byte == 0xff) {
            start_zero_idx = -1;
            cur_idx += 8;
        } else {
            int i;
            for (i = 0; i < 8; i++) {
                if (cur_byte & (1 << i)) {
                    // cur_idx bit is 1
                    start_zero_idx = -1;
                } else {
                    // cur_idx bit is 0
                    if (start_zero_idx == -1) {
                        start_zero_idx = cur_idx;
                    }
                    if (cur_idx - start_zero_idx + 1 >= cnt ) {
                        return start_zero_idx;
                    }
                }
                cur_idx++;
            }
        }

    }
    return -1;
}


void bitmap_set(struct bitmap* btmp, uint32_t bit_idx, int8_t value) {
    //ASSERT((value == 0 || value == 1));
    uint32_t byte_idx = bit_idx / 8;
    uint32_t bit_idx_in_byte = bit_idx % 8;
    int8_t op = 1 << bit_idx_in_byte;
    if (value) {
        btmp->bits[byte_idx] |= op;
    } else {
        btmp->bits[byte_idx] &= (~op);
    }
}