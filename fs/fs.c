#include "fs.h"
#include "super_block.h"
#include "dir.h"
#include "inode.h"
#include "../device/ide.h"
#include "../lib/stdint.h"
#include "../lib/common.h"
#include "../lib/string.h"
#include "../lib/kernel/io.h"

#define MAX_FILES_PER_PART  4096
#define SECTOR_SIZE         512
#define BITS_PER_SECTOR     (SECTOR_SIZE * 8)
#define BLOCK_SIZE          SECTOR_SIZE

enum file_type {
    FT_UNKNOWN,
    FT_REGULAR,
    FT_DIR
};

extern struct list_node partition_list;

/** find max value */
static uint32_t max(uint32_t a, uint32_t b, uint32_t c);
static create_filesys_for_part(struct list_node* node);

void filesys_init() {
    list_traverse(&partition_list, create_filesys_for_part);
}

static create_filesys_for_part(struct list_node* node) {
    struct partition* part = field_to_struct_ptr(struct partition, hook, node);
    struct super_block sb;
    // read in super block
    ide_read(part->my_disk, part->start_lba + 1, sb, 1);
    if (sb.magic == 0x20200303) {
        printk("  %s already has file system.\n", part->name);
    } else {
        printk("  formatting partition %s.....\n", part->name);
        partition_format(part);
    }
}

static void partition_format(struct partition* part) {
    printk("format partition %s, start\n", part->name);
    // format boot sector, super block, block bitmap, inode bitmap, inode table, data area
    uint32_t boot_sector_sec_cnt = 1;
    uint32_t super_block_sec_cnt = 1;
    uint32_t inode_bitmap_sec_cnt = CEIL(MAX_FILES_PER_PART, BITS_PER_SECTOR);
    uint32_t inode_table_sec_cnt = CEIL(MAX_FILES_PER_PART * sizeof(struct inode));

    uint32_t used_sec_cnt = boot_sector_sec_cnt + super_block_sec_cnt + inode_bitmap_sec_cnt + inode_table_sec_cnt;
    uint32_t free_sec_cnt = part->sec_cnt - used_sec_cnt;

    uint32_t block_bitmap_sec_cnt = CEIL(free_sec_cnt, BITS_PER_SECTOR);
    uint32_t block_bitmap_bit_len = free_sec_cnt - block_bitmap_sec_cnt;
    block_bitmap_sec_cnt = CEIL(block_bitmap_bit_len, BITS_PER_SECTOR);

    /* init super block */
    struct super_block sb;
    sb.magic = 0x20200303;
    sb.sec_cnt = part->sec_cnt;
    sb.inode_cnt = MAX_FILES_PER_PART;
    sb.part_lba_base = part->start_lba;

    sb.block_bitmap_lba = sb.part_lba_base + 2;
    sb.block_bitmap_sec_cnt = block_bitmap_sec_cnt;
    sb.inode_bitmap_lba = sb.block_bitmap_lba + sb.block_bitmap_sec_cnt;
    sb.inode_bitmap_sec_cnt = inode_bitmap_sec_cnt;
    sb.inode_table_lba = sb.inode_bitmap_lba + sb.inode_bitmap_sec_cnt;
    sb.inode_table_sec_cnt = inode_table_sec_cnt;
    sb.data_start_lba = sb.inode_table_lba + sb.inode_table_sec_cnt;

    sb.root_i_no = 0;
    sb.dir_entry_size = sizeof(struct dir_entry);
    print_super_block(part, &sb);

    struct disk* hd = part->my_disk;

    /* 1. write super_block to 1st sector */
    ide_write(hd, part->start_lba + 1, &sb, 1);
    printk("write super block finished");

    uint32_t buf_size = max(sb.block_bitmap_sec_cnt, sb.inode_bitmap_sec_cnt, sb.inode_table_sec_cnt);
    uint8_t* buf = (uint8_t*)sys_malloc(buf_size);

    /* 2. init block bitmap & write into sb.block_bitmap_lba */
    memset(buf, 0, buf_size);
    buf[0] = 1; // 0th block is reserved for root directory
    uint32_t last_byte = block_bitmap_bit_len / 8;
    uint32_t last_bit = block_bitmap_bit_len % 8;
    uint32_t last_size = SECTOR_SIZE - (last_byte % SECTOR_SIZE);
    memset(&buf[last_byte], 0xff, last_size);

    for (uint32_t i = 0; i <= last_bit; i++) {
        buf[last_byte] &= ~(1 << i);
    }
    ide_write(hd, sb.block_bitmap_lba, buf, sb.block_bitmap_sec_cnt);

    /* 3. init inode bitmap & write into sb.inode_bitmap_lba */
    memset(buf, 0, buf_size);
    buf[0] = 1; // 0th inode is reserved for root directory
    ide_write(hd, sb.inode_bitmap_lba, buf, sb.inode_bitmap_sec_cnt);

    /* 4. init inode table & write into sb.inode_table_lba */
    memset(buf, 0, buf_size);
    struct inode_on_hd* i = (struct inode_on_hd*) buf;
    i->size = sb.dir_entry_size * 2; // . and ..
    i->i_no = 0;
    i->sectors[0] = sb.data_start_lba;
    ide_write(hd, sb.inode_table_lba, buf, sb.inode_table_sec_cnt);

    /* 5. write root directory into sb.data_start_lba */
    memset(buf, 0, buf_size);
    struct dir_entry* dir_p = (struct dir_entry*) buf;
    // init directory .
    strcpy(dir_p->filename, ".");
    dir_p->i_no = 0;
    dir_p->f_type = FT_DIR;
    dir_p++;
    // init directory ..
    strcpy(dir_p->filename, "..");
    dir_p->i_no = 0;
    dir_p->f_type = FT_DIR;
    ide_write(hd, sb.data_start_lba, buf, 1);

    printk("format partition %s, done\n", part->name);
    sys_free(buf);
}

static void print_super_block(struct partition* part, struct super_block* sb) {
    printk("super block of %s: \n", part->name);
    printk("  magic:0x%x\n  sec_cnt:0x%x\n  inode_cnt:0x%x\n  part_lba_base:0x%x\n",
            sb->magic, sb->sec_cnt, sb->inode_cnt, sb->part_lba_base);
    printk("  block_bitmap_lba:0x%x\n  block_bitmap_sec_cnt:0x%x\n", sb->block_bitmap_lba, sb->block_bitmap_sec_cnt);
    printk("  inode_bitmap_lba:0x%x\n  inode_bitmap_sec_cnt:0x%x\n", sb->inode_bitmap_lba, sb->inode_bitmap_sec_cnt);
    printk("  inode_table_lba:0x%x\n  inode_table_sec_cnt:0x%x\n", sb->inode_table_lba, sb->inode_table_sec_cnt);
    printk("  data_start_lba:0x%x\n  root_i_no:%x\n  dir_entry_size:0x%x\n",
            sb->data_start_lba, sb->root_i_no, sb->dir_entry_size);
}

static uint32_t max(uint32_t a, uint32_t b, uint32_t c) {
    uint32_t m = (a > b) ? a : b;
    m = (m > c) ? m : c;
    return m;
}