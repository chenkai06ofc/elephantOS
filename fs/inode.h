#ifndef __FS_INODE_H
#define __FS_INODE_H

#include "../lib/stdint.h"
#include "../lib/common.h"
#include "../lib/kernel/list.h"

#define INODE_BLOCK_IDX_LEN     13

/** inode structure in memory */
struct inode {
    uint32_t i_no;
    // file size for normal files, dir entry size for directory
    uint32_t size;

    uint32_t open_cnts;
    bool write_deny;

    uint32_t sectors[INODE_BLOCK_IDX_LEN];
    struct list_node hook;
};

/** inode struct for write to & read from hard disk */
struct inode_on_hd {
    /* 64 bytes */
    uint32_t i_no;
    uint32_t size;
    uint32_t sectors[INODE_BLOCK_IDX_LEN];
    uint8_t pad[4];
} __attribute__ ((packed));


static inline void mem_to_hd(struct inode* i_p, struct inode_on_hd* h_p) {
    h_p->i_no = i_p->i_no;
    h_p->size = i_p->size;
    for (int i = 0; i < INODE_BLOCK_IDX_LEN; i++) {
        h_p->sectors[i] = i_p->sectors[i];
    }
}

static inline void hd_to_mem(struct inode_on_hd* h_p, struct inode* i_p) {
    i_p->i_no = h_p->i_no;
    i_p->size = h_p->size;
    i_p->open_cnts = 0;
    i_p->write_deny = false;
    for (int i = 0; i < INODE_BLOCK_IDX_LEN; i++) {
        i_p->sectors[i] = h_p->sectors[i];
    }
    list_init(&i_p->hook);
}

#endif //__FS_INODE_H
