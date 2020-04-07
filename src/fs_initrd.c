#include "module.h"
#include "file.h"
#include "vfs.h"
#include <string.h>

extern char initrd_init_start[];
extern char initrd_init_end[];
extern char initrd_sh_start[];
extern char initrd_sh_end[];
extern char initrd_ls_start[];
extern char initrd_ls_end[];

int fs_initrd_init(void);
void fs_initrd_exit(void);
struct file *fs_initrd_get_file(struct mountpoint *, const char *);

struct file *fs_initrd_root_file;
struct file *fs_initrd_init_file;
struct file *fs_initrd_sh_file;
struct file *fs_initrd_ls_file;

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

    fs_initrd_root_file = file_file_new();
    if (fs_initrd_root_file) {
        file_init_directory(fs_initrd_root_file, mp, 0);
    }

    fs_initrd_init_file = file_file_new();
    if (fs_initrd_init_file) {
        file_init_plain(fs_initrd_init_file, mp, 1);
    }
    
    fs_initrd_sh_file = file_file_new();
    if (fs_initrd_sh_file) {
        file_init_plain(fs_initrd_sh_file, mp, 2);
    }
    
    fs_initrd_ls_file = file_file_new();
    if (fs_initrd_ls_file) {
        file_init_plain(fs_initrd_ls_file, mp, 3);
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

    if (strcmp(pathname, "") == 0) {
        file_ptr = fs_initrd_root_file;
    } else if (strcmp(pathname, "init") == 0) {
        file_ptr = fs_initrd_init_file;
    } else if (strcmp(pathname, "sh") == 0) {
        file_ptr = fs_initrd_sh_file;
    } else if (strcmp(pathname, "ls") == 0) {
        file_ptr = fs_initrd_ls_file;
    }

    return file_ptr;
}

ssize_t fs_initrd_read(struct file *file_ptr, char *buf, size_t count, unsigned long pos) {
    char *start = NULL;
    char *end = NULL;

    if (file_ptr->plain.inode == 1) {
        start = initrd_init_start;
        end = initrd_init_end;
    } else if (file_ptr->plain.inode == 2) {
        start = initrd_sh_start;
        end = initrd_sh_end;
    } else if (file_ptr->plain.inode == 3) {
        start = initrd_ls_start;
        end = initrd_ls_end;
    }

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
    if (file_ptr != fs_initrd_root_file) {
        return -1;
    }

    switch (count) {
    case 0:
        dirp->d_ino = count + 1;
        strcpy(dirp->d_name, "init");
        return 1;
    case 1:
        dirp->d_ino = count + 1;
        strcpy(dirp->d_name, "sh");
        return 1;
    case 2:
        dirp->d_ino = count + 1;
        strcpy(dirp->d_name, "ls");
        return 0;
    default:
        return -1;
    }
}
