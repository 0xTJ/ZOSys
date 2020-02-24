#ifndef INCLUDE_DEVICE_H
#define INCLUDE_DEVICE_H

#include "file.h"
#include <stdint.h>
#include <sys/types.h>

#define DEVICE_MAJOR_NUM 8
#define DEVICE_MINOR_NUM 8

typedef int (*device_char_open_func)(struct file *file_ptr, int flags);
typedef int (*device_char_close_func)(struct file *file_ptr);
typedef ssize_t (*device_char_read_func)(struct file *file_ptr, char *buf, size_t count, unsigned long pos);
typedef ssize_t (*device_char_write_func)(struct file *file_ptr, const char *buf, size_t count, unsigned long pos);

extern struct file_ops *device_drivers[DEVICE_MAJOR_NUM];

int device_register_driver(int major, struct file_ops *drv);

int device_open(struct file *file_ptr, int flags);
int device_close(struct file *file_ptr);
ssize_t device_read(struct file *file_ptr, char *buf, size_t count, unsigned long pos);
ssize_t device_write(struct file *file_ptr, const char *buf, size_t count, unsigned long pos);

#endif
