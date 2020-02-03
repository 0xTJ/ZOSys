#ifndef INCLUDE_VFS_H
#define INCLUDE_VFS_H

#include "file.h"

struct filesystem {
    struct file *(*get_file)(struct filesystem *, const char *);
};

int vfs_mount(struct filesystem *fs, char mountpoint);
void vfs_unmount(char mountpoint);
struct filesystem *vfs_get_fs(const char *pathname);

#endif
