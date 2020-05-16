#include "module.h"
#include "file.h"
#include "vfs.h"
#include <string.h>

struct initrd_entry {
    const char *name;
    struct file *file_ptr;
    const char *start;
    const char *end;
};

extern char initrd_init_start[];
extern char initrd_init_end[];
extern char initrd_sh_start[];
extern char initrd_sh_end[];
extern char initrd_ls_start[];
extern char initrd_ls_end[];
extern char initrd_dd_start[];
extern char initrd_dd_end[];
extern char initrd_ioctl_start[];
extern char initrd_ioctl_end[];

int fs_initrd_init(void);
void fs_initrd_exit(void);
struct file *fs_initrd_get_file(struct mountpoint *, const char *);

struct initrd_entry entries[] = {
    { "", NULL, NULL, NULL },
    { "init", NULL, initrd_init_start, initrd_init_end },
    { "sh", NULL, initrd_sh_start, initrd_sh_end },
    { "ls", NULL, initrd_ls_start, initrd_ls_end },
    { "dd", NULL, initrd_dd_start, initrd_dd_end },
    { "ioctl", NULL, initrd_ioctl_start, initrd_ioctl_end },
};

struct module fs_initrd_module = {
    fs_initrd_init,
    fs_initrd_exit
};

struct file_ops fs_initrd_ops = {
    .read = fs_initrd_read,
    .readdirent = fs_initrd_readdirent
};

struct filesystem fs_initrd = {
    fs_initrd_get_file,
    &fs_initrd_ops
};

int fs_initrd_init(void) {
    struct mountpoint *mp = vfs_mount(&fs_initrd, NULL, 'Y');
    if (!mp) {
        return -1;
    }

    entries[0].file_ptr = file_file_new();
    if (entries[0].file_ptr) {
        file_init_directory(entries[0].file_ptr, mp, 0);
    }
    for (ino_t i = 1; i < (sizeof(entries) / sizeof(entries[0])); ++i) {
        entries[i].file_ptr = file_file_new();
        if (entries[i].file_ptr) {
            file_init_plain(entries[i].file_ptr, mp, i);
        }
    }

    return 0;
}

void fs_initrd_exit(void) {
    // TODO: Clean up
    return;
}

struct file *fs_initrd_get_file(struct mountpoint *mp, const char *pathname) {
    (void) mp;

    struct file *file_ptr = NULL;

    for (ino_t i = 0; i < (sizeof(entries) / sizeof(entries[0])); ++i) {
        if (strcmp(pathname, entries[i].name) == 0) {
            file_ptr = entries[i].file_ptr;
            break;
        }
    }

    if (file_ptr) {
        file_file_ref(file_ptr);
    }

    return file_ptr;
}

ssize_t fs_initrd_read(struct file *file_ptr, char *buf, size_t count, unsigned long pos) {
    const char *start = entries[file_ptr->plain.inode].start;
    const char *end = entries[file_ptr->plain.inode].end;

    if (start && end) {
        size_t file_size = end - start;

        if (pos >= file_size) {
            return -1;
        } else if (pos + count > file_size) {
            count = file_size - pos;
        }

        memcpy(buf, start + pos, count);
    
        return count;
    }

    return -1;
}

int fs_initrd_readdirent(struct file *file_ptr, struct dirent *dirp, unsigned int count) {
    if (file_ptr != entries[0].file_ptr) {
        return -1;
    }

    ino_t inode = count + 1;

    if (inode > 0 && inode < (sizeof(entries) / sizeof(entries[0]))) {
        dirp->d_ino = inode;
        strcpy(dirp->d_name, entries[inode].name);
        if (inode == (sizeof(entries) / sizeof(entries[0]) - 1)) {
            return 0;
        } else {
            return 1;
        }
    } else {
        return -1;
    } 
}
