#ifndef __FS_SUPER_BLOCK_H
#define __FS_SUPER_BLOCK_H

#include "../lib/stdint.h"

struct super_block {
    uint32_t magic;

    uint32_t sec_cnt;               // sector count
    uint32_t inode_cnt;             // inode count
    uint32_t part_lba_base;         // start lba of this partition

    uint32_t block_bitmap_lba;      // start lba of block bitmap
    uint32_t block_bitmap_sec_cnt;  // sector count of block bitmap

    uint32_t inode_bitmap_lba;      // start lba of inode bitmap
    uint32_t inode_bitmap_sec_cnt;  // sector count of inode bitmap

    uint32_t inode_table_lba;       // start lba of inode table
    uint32_t inode_table_sec_cnt;   // sector count of inode table

    uint32_t data_start_lba;        // start lba of data area
    uint32_t root_i_no;             // inode number of root directory
    uint32_t dir_entry_size;        // size of directory entry

    uint8_t pad[460];
} __attribute__ ((packed));

#endif //__FS_SUPER_BLOCK_H
