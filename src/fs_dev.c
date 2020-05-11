#include "module.h"
#include "file.h"
#include "vfs.h"
#include <string.h>

int fs_dev_init(void);
void fs_dev_exit(void);
struct file *fs_dev_get_file(struct mountpoint *, const char *);

struct file *fs_dev_root_file;
struct file *fs_dev_asci0_file;
struct file *fs_dev_asci1_file;
struct file *fs_dev_sermem_file;
struct file *fs_dev_sd0_file;
struct file *fs_dev_flash0_file;

struct module fs_dev_module = {
    fs_dev_init,
    fs_dev_exit
};

struct file_ops fs_dev_ops = {
    .readdirent = fs_dev_readdirent
};

struct filesystem fs_dev = {
    fs_dev_get_file,
    &fs_dev_ops
};

int fs_dev_init(void) {
    struct mountpoint *mp = vfs_mount(&fs_dev, NULL, 'Z');
    if (!mp) {
        return -1;
    }

    fs_dev_root_file = file_file_new();
    if (fs_dev_root_file) {
        file_init_directory(fs_dev_root_file, mp, 0);
    }

    fs_dev_asci0_file = file_file_new();
    if (fs_dev_asci0_file) {
        file_init_special(fs_dev_asci0_file, 1, 0, NULL);
    }
    
    fs_dev_asci1_file = file_file_new();
    if (fs_dev_asci1_file) {
        file_init_special(fs_dev_asci1_file, 1, 1, NULL);
    }

    fs_dev_sermem_file = file_file_new();
    if (fs_dev_sermem_file) {
        file_init_special(fs_dev_sermem_file, 2, 0, fs_dev_asci0_file);
    }

    fs_dev_sd0_file = file_file_new();
    if (fs_dev_sd0_file) {
        file_init_special(fs_dev_sd0_file, 3, 0, NULL);
    }

    fs_dev_flash0_file = file_file_new();
    if (fs_dev_flash0_file) {
        file_init_special(fs_dev_flash0_file, 4, 0, NULL);
    }

    return 0;
}

void fs_dev_exit(void) {
    // TODO: Clean up
    return;
}

struct file *fs_dev_get_file(struct mountpoint *mp, const char *pathname) {
    (void) mp;

    struct file *file_ptr = NULL;

    if (strcmp(pathname, "") == 0) {
        file_ptr = fs_dev_root_file;
    } else if (strcmp(pathname, "asci0") == 0) {
        file_ptr = fs_dev_asci0_file;
    } else if (strcmp(pathname, "asci1") == 0) {
        file_ptr = fs_dev_asci1_file;
    } else if (strcmp(pathname, "sermem0") == 0) {
        file_ptr = fs_dev_sermem_file;
    } else if (strcmp(pathname, "sd0") == 0) {
        file_ptr = fs_dev_sd0_file;
    } else if (strcmp(pathname, "flash0") == 0) {
        file_ptr = fs_dev_flash0_file;
    }

    if (file_ptr) {
        file_file_ref(file_ptr);
    }

    return file_ptr;
}

int fs_dev_readdirent(struct file *file_ptr, struct dirent *dirp, unsigned int count) {
    if (file_ptr != fs_dev_root_file) {
        return -1;
    }

    switch (count) {
    case 0:
        dirp->d_ino = count + 1;
        strcpy(dirp->d_name, "asci0");
        return 1;
    case 1:
        dirp->d_ino = count + 1;
        strcpy(dirp->d_name, "asci1");
        return 1;
    case 2:
        dirp->d_ino = count + 1;
        strcpy(dirp->d_name, "sermem0");
        return 1;
    case 3:
        dirp->d_ino = count + 1;
        strcpy(dirp->d_name, "sd0");
        return 1;
    case 4:
        dirp->d_ino = count + 1;
        strcpy(dirp->d_name, "flash0");
        return 0;
    default:
        return -1;
    }
}
