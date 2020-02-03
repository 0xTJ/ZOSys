#include "fs_dev.h"
#include <string.h>

#include "asci.h"

extern struct device_char *asci_0;

struct file *fs_dev_get_file(struct filesystem *, const char *);

struct filesystem fs_dev = {
    fs_dev_get_file
};

int fs_dev_init(void) {
    return vfs_mount(&fs_dev, 'Z');
}

struct file *fs_dev_get_file(struct filesystem *fs, const char *pathname) {
    struct file *file = NULL;
    if (strcmp(pathname, "asci0") == 0) {
        file = file_file_new();
        if (file) {
            file->type = FILE_CHAR_DEV;
            file->dev_char = asci_0;
        }
    }
    return file;
}
