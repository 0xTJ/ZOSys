#include "module.h"
#include "file.h"
#include "vfs.h"
#include <string.h>

int fs_dev_init(void);
void fs_dev_exit(void);
struct file *fs_dev_get_file(struct filesystem *, const char *);

struct module fs_dev_module = {
    fs_dev_init,
    fs_dev_exit
};

struct filesystem fs_dev = {
    fs_dev_get_file
};

int fs_dev_init(void) {
    return vfs_mount(&fs_dev, 'Z');
}

void fs_dev_exit(void) {
    // TODO: Clean up
    return;
}

struct file *fs_dev_get_file(struct filesystem *fs, const char *pathname) {
    struct file *file = NULL;
    if (strcmp(pathname, "asci0") == 0) {
        file = file_file_new();
        if (file) {
            file->type = FILE_SPECIAL;
            file->special.major = 1;
            file->special.minor = 0;
        }
    }
    return file;
}
