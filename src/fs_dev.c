#include "module.h"
#include "file.h"
#include "vfs.h"
#include <string.h>

int fs_dev_init(void);
void fs_dev_exit(void);
struct file *fs_dev_get_file(struct mountpoint *, const char *);

struct module fs_dev_module = {
    fs_dev_init,
    fs_dev_exit
};

struct filesystem fs_dev = {
    fs_dev_get_file,
    NULL
};

int fs_dev_init(void) {
    return vfs_mount(&fs_dev, NULL, 'Z');
}

void fs_dev_exit(void) {
    // TODO: Clean up
    return;
}

struct file *fs_dev_get_file(struct mountpoint *mp, const char *pathname) {
    struct file *file_ptr = NULL;
    if (strcmp(pathname, "") == 0) {
        file_ptr = file_file_new();
        if (file_ptr) {
            file_init_directory(file_ptr, mp, 0);
        }
    } else if (strcmp(pathname, "asci0") == 0) {
        file_ptr = file_file_new();
        if (file_ptr) {
            file_init_special(file_ptr, 1, 0);
        }
    }
    return file_ptr;
}
