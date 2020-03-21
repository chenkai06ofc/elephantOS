#ifndef __DEVICE__IDE_H
#define __DEVICE__IDE_H

#include "../lib/stdint.h"
#include "../lib/common.h"
#include "../lib/kernel/list.h"
#include "../lib/kernel/bitmap.h"
#include "../thread/sync.h"

struct partition {
    char name[8];
    uint32_t start_lba;
    uint32_t sec_cnt;
    struct disk* my_disk;
    struct list_node hook;
    struct super_block* sb;
    struct bitmap block_bitmap;
    struct bitmap inode_bitmap;
    struct list_node open_inodes;   // i-nodes opened in this partition
};

struct disk {
    char name[8];
    struct ide_channel* my_channel;
    // master (0), slave (1)
    uint8_t dev_no;
    struct partition prim_parts[4];
    struct partition logic_parts[4];
};

struct ide_channel {
    char name[8];
    uint16_t port_base;
    uint8_t vec_no;
    struct lock lock;
    bool expecting_intr;
    struct semaphore disk_done;
    struct disk devices[2];
};

struct partition_table_entry {
    uint8_t bootable;
    uint8_t start_head;
    uint8_t start_sec;
    uint8_t start_chs;
    uint8_t fs_type;
    uint8_t end_head;
    uint8_t end_sec;
    uint8_t end_chs;
    uint32_t start_lba;
    uint32_t sec_cnt;
} __attribute__ ((packed));

/** boot sector, mbr or ebr */
struct boot_sector {
    uint8_t other[446];
    struct partition_table_entry partition_table[4];
    uint16_t signature;
} __attribute__ ((packed));

void ide_init(void);

/** read sec_cnt sectors from lba to memory address buf */
void ide_read(struct disk* hd, uint32_t lba, void* buf, uint32_t sec_cnt);
/** write sec_cnt sectors from memory address buf to lba */
void ide_write(struct disk* hd, uint32_t lba, void* buf, uint32_t sec_cnt);

#endif //__DEVICE__IDE_H
