#include "device.h"

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

