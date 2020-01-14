#ifndef INCLUDE_DEVICE_H
#define INCLUDE_DEVICE_H

#include "mutex.h"
#include <stdint.h>
#include <sys/types.h>

enum device_type {
    DEVICE_CHARACTER,
    DEVICE_BLOCK
};

struct device {
    mutex_t mtx;
    enum device_type type;
    union {
        struct {
            struct device_char_driver *driver;
            union {
                unsigned int data_ui;
                void *data_p;
            };
        } character;
        struct {
            struct device_block_driver *driver;
            unsigned int block_size;
        } block;
    };
    union {
        unsigned int data_ui;
        void *data_p;
    };
};

typedef int (*device_open_func)(struct device *dev, int flags);
typedef int (*device_close_func)(struct device *dev);

typedef ssize_t (*device_char_read_func)(struct device *dev, char *buf, size_t count, unsigned long pos);
typedef ssize_t (*device_char_write_func)(struct device *dev, const char *buf, size_t count, unsigned long pos);

typedef ssize_t (*device_block_read_func)(struct device *dev, char *buf, unsigned int block_count, unsigned long pos);
typedef ssize_t (*device_block_write_func)(struct device *dev, const char *buf, unsigned int block_count, unsigned long pos);

struct device_char_driver {
    device_open_func open;
    device_close_func close;
    device_char_read_func read;
    device_char_write_func write;
};

struct device_block_driver {
    device_open_func open;
    device_close_func close;
    device_block_read_func read;
    device_block_write_func write;
};

// Returns with mutex locked, must be unlocked after setup
struct device *device_char_new(struct device_char_driver *driver);
struct device *device_block_new(struct device_block_driver *driver, unsigned int block_size);

int device_open(struct device *dev, int flags);
int device_char_close(struct device *dev);
ssize_t device_char_read(struct device *dev, char *buf, size_t count, unsigned long pos);
ssize_t device_char_write(struct device *dev, const char *buf, size_t count, unsigned long pos);
ssize_t device_block_read(struct device *dev, char *buf, unsigned int block_count, unsigned long pos);
ssize_t device_block_write(struct device *dev, const char *buf, unsigned int block_count, unsigned long pos);

ssize_t dummy_open(struct device *dev, int flags);
ssize_t dummy_close(struct device *dev);
ssize_t dummy_char_read(struct device *dev, char *buf, size_t count, unsigned long pos);
ssize_t dummy_char_write(struct device *dev, const char *buf, size_t count, unsigned long pos);
ssize_t dummy_block_read(struct device *dev, char *buf, unsigned int block_count, unsigned long pos);
ssize_t dummy_block_write(struct device *dev, const char *buf, unsigned int block_count, unsigned long pos);

#endif
