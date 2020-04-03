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

struct module fs_dev_module = {
    fs_dev_init,
    fs_dev_exit
};

struct filesystem fs_dev = {
    fs_dev_get_file,
    NULL
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

    return 0;
}

void fs_dev_exit(void) {
    // TODO: Clean up
    return;
}

struct file *fs_dev_get_file(struct mountpoint *mp, const char *pathname) {
    struct file *file_ptr = NULL;
    if (strcmp(pathname, "") == 0) {
        file_ptr = fs_dev_root_file;
    } else if (strcmp(pathname, "asci0") == 0) {
        file_ptr = fs_dev_asci0_file;
    } else if (strcmp(pathname, "asci1") == 0) {
        file_ptr = fs_dev_asci1_file;
    } else if (strcmp(pathname, "sermem0") == 0) {
        file_ptr = fs_dev_sermem_file;
    }
    return file_ptr;
}
