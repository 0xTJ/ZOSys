#ifndef INCLUDE_DEVICE_H
#define INCLUDE_DEVICE_H

#include "mutex.h"
#include <stdint.h>
#include <sys/types.h>

struct device_char {
    mutex_t mtx;
    struct device_char_driver *driver;
    union {
        unsigned int data_ui;
        void *data_p;
    };
};

struct device_block {
    mutex_t mtx;
    struct device_block_driver *driver;
    unsigned int block_size;
    union {
        unsigned int data_ui;
        void *data_p;
    };
};

typedef int (*device_char_open_func)(struct device_char *dev, int flags);
typedef int (*device_char_close_func)(struct device_char *dev);
typedef ssize_t (*device_char_read_func)(struct device_char *dev, char *buf, size_t count, unsigned long pos);
typedef ssize_t (*device_char_write_func)(struct device_char *dev, const char *buf, size_t count, unsigned long pos);

typedef int (*device_block_open_func)(struct device_block *dev, int flags);
typedef int (*device_block_close_func)(struct device_block *dev);
typedef ssize_t (*device_block_read_func)(struct device_block *dev, char *buf, unsigned int block_count, unsigned long pos);
typedef ssize_t (*device_block_write_func)(struct device_block *dev, const char *buf, unsigned int block_count, unsigned long pos);

struct device_char_driver {
    device_char_open_func open;
    device_char_close_func close;
    device_char_read_func read;
    device_char_write_func write;
};

struct device_block_driver {
    device_block_open_func open;
    device_block_close_func close;
    device_block_read_func read;
    device_block_write_func write;
};

// Returns with mutex locked, must be unlocked after setup
struct device_char *device_char_new(struct device_char_driver *driver);
struct device_block *device_block_new(struct device_block_driver *driver, unsigned int block_size);

int device_char_open(struct device_char *dev, int flags);
int device_char_close(struct device_char *dev);
ssize_t device_char_read(struct device_char *dev, char *buf, size_t count, unsigned long pos);
ssize_t device_char_write(struct device_char *dev, const char *buf, size_t count, unsigned long pos);

int device_block_open(struct device_block *dev, int flags);
int device_block_close(struct device_block *dev);
ssize_t device_block_read(struct device_block *dev, char *buf, unsigned int block_count, unsigned long pos);
ssize_t device_block_write(struct device_block *dev, const char *buf, unsigned int block_count, unsigned long pos);

ssize_t dummy_char_open(struct device_char *dev, int flags);
ssize_t dummy_char_close(struct device_char *dev);
ssize_t dummy_char_read(struct device_char *dev, char *buf, size_t count, unsigned long pos);
ssize_t dummy_char_write(struct device_char *dev, const char *buf, size_t count, unsigned long pos);

ssize_t dummy_block_open(struct device_block *dev, int flags);
ssize_t dummy_block_close(struct device_block *dev);
ssize_t dummy_block_read(struct device_block *dev, char *buf, unsigned int block_count, unsigned long pos);
ssize_t dummy_block_write(struct device_block *dev, const char *buf, unsigned int block_count, unsigned long pos);

#endif
