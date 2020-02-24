#include "module.h"
#include "file.h"
#include "vfs.h"
#include <string.h>

extern char initrd_init_start[];
extern char initrd_init_end[];

int fs_initrd_init(void);
void fs_initrd_exit(void);
struct file *fs_initrd_get_file(struct mountpoint *, const char *);

struct module fs_initrd_module = {
    fs_initrd_init,
    fs_initrd_exit
};

struct file_ops fs_initrd_ops = {
    .read = fs_initrd_read
};

struct filesystem fs_initrd = {
    fs_initrd_get_file,
    &fs_initrd_ops
};

int fs_initrd_init(void) {
    return vfs_mount(&fs_initrd, NULL, 'Y');
}

void fs_initrd_exit(void) {
    // TODO: Clean up
    return;
}

struct file *fs_initrd_get_file(struct mountpoint *mp, const char *pathname) {
    (void) mp;

    struct file *file_ptr = NULL;
    if (strcmp(pathname, "init") == 0) {
        file_ptr = file_file_new();
        if (file_ptr) {
            file_init_plain(file_ptr, mp, 0);
        }
    }
    return file_ptr;
}

ssize_t fs_initrd_read(struct file *file_ptr, char *buf, size_t count, unsigned long pos) {
    char *start = NULL;
    char *end = NULL;

    if (file_ptr->plain.inode == 0) {
        start = initrd_init_start;
        end = initrd_init_end;
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