#ifndef INCLUDE_VFS_H
#define INCLUDE_VFS_H

#include "file.h"
#include "device.h"

struct mountpoint {
    struct filesystem *fs;
    struct file *backing;
};

struct filesystem {
    struct file *(*get_file)(struct mountpoint *, const char *);
    struct file_ops *ops;
};

struct mountpoint *vfs_mount(struct filesystem *fs, struct file *backing, char mountpoint);
void vfs_unmount(char mountpoint);
struct mountpoint *vfs_get_mount(const char *pathname);

#endif
