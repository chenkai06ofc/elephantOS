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
    uint8_t dev_no;
    struct partition prim_parts[4];
    struct partition logic_parts[4];
};

struct ide_channel {
    char name[8];
    uint16_t port_base;
    uint8_t irq_no;
    struct lock lock;
    bool expecting_intr;
    struct semaphore disk_done;
    struct disk devices[2];
};

#endif //__DEVICE__IDE_H
