#ifndef __LIB_IO_H
#define __LIB_IO_H
#include "stdint.h"

/** output 1 byte into port */
static inline void outb(uint16_t port, uint8_t data) {
    asm volatile ("outb %b0, %w1" : : "a"(data), "Nd" (port));
}

/** output word_cnt words start from addr into port */
static inline void outsw(uint16_t port, const void* addr, uint32_t word_cnt) {
    // outsw write the 16-bit contents in ds:esi into port
    asm volatile ("cld; rep outsw" : : "S" (addr), "c" (word_cnt), "d" (port));
}

/** input 1 byte from port */
static inline uint8_t inb(uint16_t port) {
    uint8_t data;
    asm volatile ("inb %w1, %b0" : "=a" (data) : "Nd" (port));
    return data;
}

/** input word_cnt bytes from port into addr */
static inline void insw(uint16_t port, void* addr, uint32_t word_cnt) {
    // insw read 16-bit contents from port into es:edi
    asm volatile ("cld; rep insw" : : "D" (addr), "c" (word_cnt), "d" (port) : "memory");
}

#endif //__LIB_IO_H
