#include "ide.h"
#include "../kernel/debug.h"
#include "../kernel/interrupt.h"
#include "../thread/thread.h"
#include "../lib/common.h"
#include "../lib/string.h"
#include "../lib/stdio.h"
#include "../lib/kernel/io.h"
#include "../lib/kernel/list.h"

/** IO ports of channel registers */
#define reg_data(channel)           (channel->port_base + 0)
#define reg_error(channel)          (channel->port_base + 1)
#define reg_sect_cnt(channel)       (channel->port_base + 2)
#define reg_lba_low(channel)        (channel->port_base + 3)
#define reg_lba_middle(channel)     (channel->port_base + 4)
#define reg_lba_high(channel)       (channel->port_base + 5)
#define reg_dev(channel)            (channel->port_base + 6)
#define reg_status(channel)         (channel->port_base + 7)
#define reg_cmd(channel)            (channel->port_base + 7)
#define reg_alt_status(channel)     (channel->port_base + 0x206)
#define reg_ctl(channel)            (channel->port_base + 0x206)

/** some bits of reg_alt_status register */
#define BIT_ALT_STAT_BUSY           0x80
#define BIT_ALT_STAT_READY          0x40
#define BIT_ALT_STAT_DRQ            0x8

/** some bits of dev register */
#define BIT_DEV_MBS         0xa0
#define BIT_DEV_LBA         0x40
#define BIT_DEV_SLAVE       0x10

/** some commands for hd operations */
#define CMD_IDENTIFY            0xec
#define CMD_READ_SECTOR         0x20
#define CMD_WRITE_SECTOR        0x30

#define IDE0_VEC_NO             0x2e
#define IDE1_VEC_NO             0x2f

#define PART_FS_TYPE_EXT        0x5

/** list of partition */
struct list_node partition_list;
/** list of channels */
struct ide_channel channels[2];

/** init a channel with pre-defined values */
static void ide_channel_init(struct ide_channel* channel, char* name, uint16_t port_base, uint8_t vec_no);

/** these functions are somehow direct wrapper of IO port operations */
static void select_disk(struct disk* hd);
static void select_sector(struct disk* hd, uint32_t lba, uint8_t sec_cnt);
static void cmd_out(struct ide_channel* channel, uint8_t cmd);
static void read_sector(struct disk* hd, void* buf, uint8_t sec_cnt);
static void write_to_sector(struct disk* hd, void* buf, uint8_t sec_cnt);
static bool busy_wait(struct disk* hd);

static void hd_init(struct ide_channel* channel, uint8_t idx, char* name);
/** acquire disk parameters */
static void identify_disk(struct disk* hd);
/** scan hard disk and get info of all partitions */
static void partition_scan(struct disk* hd, uint32_t ext_lba, bool is_ext);
/** print info of the partition */
static void partition_info(struct list_node* node);

/** hard disk interrupt handler */
static void hd_intr_handler(uint8_t vec_no);
/** swap adjacent 2 bytes in dst then put in buf */
static void swap_pairs_bytes(const char* dst, char* buf, uint32_t len);

void ide_init(void) {
    printk("ide_init start\n");
    uint8_t hd_cnt = *((uint8_t*)(0x475));
    ASSERT(hd_cnt > 0);
    uint8_t channel_cnt = (hd_cnt > 2) ? 2 : 1;

    // init channels
    ide_channel_init(&channels[0], "ide0", 0x1f0, IDE0_VEC_NO);
    register_intr_handler(channels[0].vec_no, hd_intr_handler);
    if (channel_cnt > 1) {
        ide_channel_init(&channels[1], "ide1", 0x170, IDE1_VEC_NO);
        register_intr_handler(channels[1].vec_no, hd_intr_handler);
    }

    // init hard disks
    char* hd_names[] = { "sda", "sdb", "sdc", "sdd" };
    for (int i = 0; i < hd_cnt; i++) {
        hd_init(&channels[i / 2], i % 2, hd_names[i]);
    }
    list_traverse(&partition_list, partition_info);
    printk("ide_init done\n");
}

void ide_read(struct disk* hd, uint32_t lba, void* buf, uint32_t sec_cnt) {
    ASSERT(sec_cnt > 0);
    lock_acquire(&hd->my_channel->lock);

    uint32_t sec_op;
    uint32_t sec_done = 0;
    while (sec_done < sec_cnt) {
        sec_op = ((sec_done + 255) <= sec_cnt) ? 255 : sec_cnt - sec_done;
        select_sector(hd, lba + sec_done, sec_op);
        cmd_out(hd->my_channel, CMD_READ_SECTOR);
        // thread block itself when hard disk starts to work, it's waked up by hd interrupt
        semaphore_down(&hd->my_channel->disk_done);

        if (!busy_wait(hd)) {
            char error[64];
            sprintf(error, "Read sector %d failed!\n", lba);
            PANIC(error);
        }

        read_sector(hd, (void*)((uint32_t)buf + sec_done * 512), sec_op);
        sec_done += sec_op;
    }
    lock_release(&hd->my_channel->lock);
}

void ide_write(struct disk* hd, uint32_t lba, void* buf, uint32_t sec_cnt) {
    ASSERT(sec_cnt > 0);
    lock_acquire(&hd->my_channel->lock);

    uint32_t sec_op;
    uint32_t sec_done = 0;
    while (sec_done < sec_cnt) {
        sec_op = ((sec_done + 255) <= sec_cnt) ? 255 : sec_cnt - sec_done;
        select_sector(hd, lba + sec_done, sec_op);
        cmd_out(hd->my_channel, CMD_WRITE_SECTOR);

        if (!busy_wait(hd)) {
            char error[64];
            sprintf(error, "Write sector %d failed!\n", lba);
            PANIC(error);
        }

        write_to_sector(hd, (void*)((uint32_t)buf + sec_done * 512), sec_op);
        // block itself until write operation is finished
        semaphore_down(&hd->my_channel->disk_done);
        sec_done += sec_op;
    }
    lock_release(&hd->my_channel->lock);
}

static void ide_channel_init(struct ide_channel* channel, char* name, uint16_t port_base, uint8_t vec_no) {
    strcpy(channel->name, name);
    channel->port_base = port_base;
    channel->vec_no = vec_no;
    channel->expecting_intr = false;
    lock_init(&channel->lock);
    semaphore_init(&channel->disk_done, 0);
}

/** length of name should < 8 */
static void hd_init(struct ide_channel* channel, uint8_t idx, char* name) {
    ASSERT(idx == 0 || idx == 1);
    struct disk* hd = &channel->devices[idx];
    hd->my_channel = channel;
    hd->dev_no = idx;
    hd->prim_parts_len = 0;
    hd->logic_parts_len = 0;
    strcpy(hd->name, name);
    identify_disk(hd);
    if (idx != 0) {
        // don't scan 0th disk
        partition_scan(hd, 0, false);
    }
}

static void hd_intr_handler(uint8_t vec_no) {
     ASSERT(vec_no == IDE0_VEC_NO || vec_no == IDE1_VEC_NO);
     struct ide_channel* channel = &channels[vec_no - IDE0_VEC_NO];
     if (channel->expecting_intr) {
         channel->expecting_intr = false;
         semaphore_up((&channel->disk_done));
         // read status register let hd controller know the interrupt has been processed
         inb(reg_status(channel));
     }
}

/** choose disk to do I/O */
static void select_disk(struct disk* hd) {
    outb(reg_dev(hd->my_channel), BIT_DEV_MBS | BIT_DEV_LBA | ((hd->dev_no == 1) ? BIT_DEV_SLAVE : 0));
}

/** specify start lba & sector count */
static void select_sector(struct disk* hd, uint32_t lba, uint8_t sec_cnt) {
    struct ide_channel* channel = hd->my_channel;
    outb(reg_sect_cnt(channel), sec_cnt);
    outb(reg_lba_low(channel), lba & 0xFF);
    outb(reg_lba_middle(channel), (lba >> 8) & 0xFF);
    outb(reg_lba_high(channel), (lba >> 16) & 0xFF);
    outb(reg_dev(channel), BIT_DEV_MBS | BIT_DEV_LBA | ((hd->dev_no == 1) ? BIT_DEV_SLAVE : 0) | (lba >> 24));
}

/** send command to channel */
static void cmd_out(struct ide_channel* channel, uint8_t cmd) {
    channel->expecting_intr = true;
    outb(reg_cmd(channel), cmd);
}

/** read sec_cnt sectors to buf */
static void read_sector(struct disk* hd, void* buf, uint8_t sec_cnt) {
    uint32_t size_in_byte = sec_cnt * 512;
    insw(reg_data(hd->my_channel), buf, size_in_byte / 2);
}

/** write sec_cnt sectors from buf to hd */
static void write_to_sector(struct disk* hd, void* buf, uint8_t sec_cnt) {
    uint32_t size_in_byte = sec_cnt * 512;
    outsw(reg_data(hd->my_channel), buf, size_in_byte / 2);
}

static bool busy_wait(struct disk* hd) {
    uint32_t time_limit = 30 * 1000; // 30 seconds
    while ((time_limit -= 10) > 0) {
        if (!(inb(reg_status(hd->my_channel)) & BIT_ALT_STAT_BUSY)) {
            return (inb(reg_status(hd->my_channel)) & BIT_ALT_STAT_DRQ);
        } else {
            sleep(10);
        }
    }
    return false;
}

static void swap_pairs_bytes(const char* dst, char* buf, uint32_t len) {
    for (int i = 0; i < len; i+=2) {
        buf[i] = dst[i + 1];
        buf[i + 1] = dst[i];
    }
    buf[len] = '\0';
}

static void identify_disk(struct disk* hd) {
    char id_info[512];
    select_disk(hd);
    cmd_out(hd->my_channel, CMD_IDENTIFY);
    semaphore_down(&hd->my_channel->disk_done);

    if (!busy_wait(hd)) {
        char error[64];
        sprintf(error, "%s identify failed!\n", hd->name);
        PANIC(error);
    }
    read_sector(hd, id_info, 1);

    char buf[64];
    uint8_t sn_start = 10 * 2, sn_len = 20, md_start = 27 * 2, md_len = 40;
    swap_pairs_bytes(&id_info[sn_start], buf, sn_len);
    printk("  disk %s info: \n    SN: %s\n", hd->name, buf);
    memset(buf, 0, sizeof(buf));
    swap_pairs_bytes(&id_info[md_start], buf, md_len);
    printk("    MODULE: %s\n", buf);
    uint32_t sectors = *((uint32_t*)&id_info[60 * 2]);
    printk("    SECTORS: %d\n", sectors);
    printk("    CAPACITY: %dMB\n", sectors * 512 / 1024 / 1024);
}

static void partition_scan(struct disk* hd, uint32_t ext_lba, bool is_ext) {
    struct boot_sector* bs = sys_malloc(sizeof(struct boot_sector));
    ide_read(hd, ext_lba, bs, 1);

    for (int i = 0; i < 4; i++) {
        struct partition_table_entry* pte = &bs->partition_table[i];

        if (pte->fs_type == PART_FS_TYPE_EXT) { // extended partition
            partition_scan(hd, ext_lba + pte->start_lba, true);
        } else if (pte->fs_type != 0) {
            uint8_t idx;
            if (is_ext) {
                idx = hd->logic_parts_len;
                hd->logic_parts[idx].start_lba = ext_lba + pte->start_lba;
                hd->logic_parts[idx].sec_cnt = pte->sec_cnt;
                hd->prim_parts[idx].my_disk = hd;
                sprintf(hd->logic_parts[idx].name, "%s%d", hd->name, idx + 5);
                list_append(&partition_list, &hd->logic_parts[idx].hook);
                hd->logic_parts_len++;
                ASSERT(hd->logic_parts_len < 4);
            } else {
                idx = hd->prim_parts_len;
                hd->prim_parts[idx].start_lba = ext_lba + pte->start_lba;
                hd->prim_parts[idx].sec_cnt = pte->sec_cnt;
                hd->prim_parts[idx].my_disk = hd;
                sprintf(hd->prim_parts[idx].name, "%s%d", hd->name, idx + 1);
                list_append(&partition_list, &hd->prim_parts[idx].hook);
                hd->prim_parts_len++;
                ASSERT(hd->prim_parts_len < 4);
            }
        }
    }
    sys_free(bs);
}

static void partition_info(struct list_node* node) {
    struct partition* part = field_to_struct_ptr(struct partition, hook, node);
    printk("  %s start_lba: 0x%x, sec_cnt: 0x%x\n", part->name, part->start_lba, part->sec_cnt);
}