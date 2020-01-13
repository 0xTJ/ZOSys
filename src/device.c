#include "device.h"
#include <adt.h>
#include <stdlib.h>

struct device *device_new(struct device_driver *driver) {
    struct device *new_dev = malloc(sizeof(struct device));

    if (!new_dev)
        return NULL;

    mutex_init(&new_dev->mtx);
    mutex_lock(&new_dev->mtx);

    new_dev->driver = driver;

    return new_dev;
}

ssize_t device_open(struct device *dev, int flags) {
    device_open_func open_func = dev->driver->open;
    if (open_func)
        return open_func(dev, flags);
    else
        return -1;
}

ssize_t device_close(struct device *dev) {
    device_close_func close_func = dev->driver->close;
    if (close_func)
        return close_func(dev);
    else
        return -1;
}

ssize_t device_read(struct device *dev, char *buf, size_t count, unsigned long pos) {
    device_read_func read_func = dev->driver->read;
    if (read_func)
        return read_func(dev, buf, count, pos);
    else
        return -1;
}

ssize_t device_write(struct device *dev, const char *buf, size_t count, unsigned long pos) {
    device_write_func write_func = dev->driver->write;
    if (write_func)
        return write_func(dev, buf, count, pos);
    else
        return -1;
}

