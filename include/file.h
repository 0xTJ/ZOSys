#ifndef INCLUDE_FILE_H
#define INCLUDE_FILE_H

#include "device.h"
#include "mem.h"
#include <stdint.h>
#include <sys/types.h>

#define MAX_OPEN_FILES 8

enum file_type {
    FILE_CHAR_DEV,
    FILE_BLOCK_DEV
};

struct file {
    enum file_type type;
    union {
        struct device_char *dev_char;
        struct device_block *dev_block;
    };
};

struct open_file {
    struct file *file;
    unsigned long pos;
};

struct file *file_open(const char *pathname, int flags);
int file_close(struct file *file);
ssize_t file_read(struct file *file, char *buf, size_t count, unsigned long pos);
ssize_t file_write(struct file *file, const char *buf, size_t count, unsigned long pos);

int sys_open(USER_PTR(char) pathname, int flags);
int sys_close(int fd);
ssize_t sys_read(int fd, USER_PTR(char) buf, size_t count);
ssize_t sys_write(int fd, USER_PTR(char) buf, size_t count);

#endif
