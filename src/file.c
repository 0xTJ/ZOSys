#include "file.h"
#include "process.h"
#include <stdlib.h>

struct file *file_open(const char *pathname, int flags) {
    return NULL;
}

int file_close(struct file *file) {
    return 0;
}

ssize_t file_read(struct file *file, char *buf, size_t count, unsigned long pos) {
    switch (file->type) {
    case FILE_CHAR_DEV:
        return device_char_read(file->dev_char, buf, count, pos);
    case FILE_BLOCK_DEV:
        return device_block_read(file->dev_block, buf, count, pos);
    default:
        return -1;
    }
}

ssize_t file_write(struct file *file, const char *buf, size_t count, unsigned long pos) {
    switch (file->type) {
    case FILE_CHAR_DEV:
        return device_char_write(file->dev_char, buf, count, pos);
    case FILE_BLOCK_DEV:
        return device_block_write(file->dev_block, buf, count, pos);
    default:
        return -1;
    }
}

int sys_open(uintptr_t pathname, int flags) {
    int found_fd = -1;
    for (found_fd = 0; found_fd < MAX_OPEN_FILES; ++found_fd)
        if (!current_proc->open_files[found_fd])
            break;
    if (found_fd == MAX_OPEN_FILES)
        return -1;

    struct open_file *open_file = malloc(sizeof(struct open_file));
    if (!open_file)
        return -1;

    open_file->file = file_open(pathname, flags);
    if (!open_file->file) {
        free(open_file);
        return -1;
    }

    open_file->pos = 0;
    current_proc->open_files[found_fd] = open_file;
    return found_fd;
}

int sys_close(int fd) {
    struct open_file *open_file = current_proc->open_files[fd];
    if (!open_file)
        return -1;
    file_close(open_file->file);
    free(open_file);
    current_proc->open_files[fd] = NULL;
    return 0;
}

ssize_t sys_read(int fd, uintptr_t buf, size_t count, unsigned long pos) {
    struct open_file *open_file = current_proc->open_files[fd];
    if (!open_file)
        return -1;
    // return file_read(open_file->file, buf, count, pos);
}

ssize_t sys_write(int fd, uintptr_t buf, size_t count, unsigned long pos) {
    struct open_file *open_file = current_proc->open_files[fd];
    if (!open_file)
        return -1;
    // return file_write(open_file->file, buf, count, pos);
}
