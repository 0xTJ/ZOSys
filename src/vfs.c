#include "vfs.h"
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>

volatile struct mountpoint mountpoints['Z' - 'A' + 1] = {{0}};

struct mountpoint *vfs_mount(struct filesystem *fs, struct file *backing, char mountdrive) __critical {
    if (!isalpha(mountdrive))
        return NULL;

    if (islower(mountdrive))
        mountdrive = toupper(mountdrive);
    uint8_t fs_num = mountdrive - 'A';

    if (mountpoints[fs_num].fs)
        return NULL;

    if (backing) {
        file_file_ref(backing);
    }

    mountpoints[fs_num].fs = fs;
    mountpoints[fs_num].backing = backing;

    return &mountpoints[fs_num];
}

void vfs_unmount(char mountdrive) __critical {
    if (!isalpha(mountdrive))
        return;

    if (islower(mountdrive))
        mountdrive = toupper(mountdrive);
    uint8_t fs_num = mountdrive - 'A';

    if (mountpoints[fs_num].backing) {
        file_file_unref(mountpoints[fs_num].backing);
    }

    mountpoints[fs_num].fs = NULL;
    mountpoints[fs_num].backing = NULL;
}

struct mountpoint *vfs_get_mount(const char *pathname) __critical {
    if (pathname[0] == '\0' || !isalpha(pathname[0]) || pathname[1] != ':')
        return NULL;

    char fs_letter = islower(pathname[0]) ? toupper(pathname[0]) : pathname[0];
    uint8_t fs_num = fs_letter - 'A';

    if (mountpoints[fs_num].fs) {
        return &mountpoints[fs_num];
    } else {
        return NULL;
    }
}
