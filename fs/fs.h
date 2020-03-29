#ifndef __FS_FS_H
#define __FS_FS_H

enum file_type {
    FT_UNKNOWN,
    FT_REGULAR,
    FT_DIR
};

void filesys_init();

#endif //__FS_FS_H
