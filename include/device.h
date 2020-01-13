#ifndef INCLUDE_DEVICE_H
#define INCLUDE_DEVICE_H

#include "mutex.h"
#include <stdint.h>
#include <sys/types.h>

struct device {
    struct device_driver *driver;
    mutex_t mtx;
    union {
        unsigned int data_ui;
        void *data_p;
    };
};

typedef ssize_t (*device_open_func)(struct device *dev, int flags);
typedef ssize_t (*device_close_func)(struct device *dev);
typedef ssize_t (*device_read_func)(struct device *dev, char *buf, size_t count, unsigned long pos);
typedef ssize_t (*device_write_func)(struct device *dev, const char *buf, size_t count, unsigned long pos);

struct device_driver {
    device_open_func open;
    device_close_func close;
    device_read_func read;
    device_write_func write;
};

// Returns with mutex locked, must be unlocked after setup
struct device *device_new(struct device_driver *driver);

ssize_t device_open(struct device *dev, int flags);
ssize_t device_close(struct device *dev);
ssize_t device_read(struct device *dev, char *buf, size_t count, unsigned long pos);
ssize_t device_write(struct device *dev, const char *buf, size_t count, unsigned long pos);

#endif
