#include "vfs.h"
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>

struct filesystem * volatile filesystems['Z' - 'A' + 1] = {0};

int vfs_mount(struct filesystem *fs, char mountpoint) __critical {
    if (!isalpha(mountpoint))
        return -1;

    if (islower(mountpoint))
        mountpoint = toupper(mountpoint);
    uint8_t fs_num = mountpoint - 'A';

    if (filesystems[fs_num])
        return -1;
    filesystems[fs_num] = fs;

    return mountpoint;
}

void vfs_unmount(char mountpoint) __critical {
    if (!isalpha(mountpoint))
        return;

    if (islower(mountpoint))
        mountpoint = toupper(mountpoint);
    uint8_t fs_num = mountpoint - 'A';

    filesystems[fs_num] = NULL;
}

struct filesystem *vfs_get_fs(const char *pathname) __critical {
    if (pathname[0] == '\0' || !isalpha(pathname[0]) || pathname[1] != ':')
        return NULL;

    char fs_letter = islower(pathname[0]) ? toupper(pathname[0]) : pathname[0];
    uint8_t fs_num = fs_letter - 'A';

    return filesystems[fs_num];
}
