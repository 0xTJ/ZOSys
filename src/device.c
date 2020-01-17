#include "device.h"
#include <adt.h>
#include <stdlib.h>

struct device_char *device_char_new(struct device_char_driver *driver) {
    struct device_char *new_dev = malloc(sizeof(struct device_char));

    if (!new_dev)
        return NULL;

    mutex_init(&new_dev->mtx);
    mutex_lock(&new_dev->mtx);

    new_dev->driver = driver;

    return new_dev;
}
struct device_block *device_block_new(struct device_block_driver *driver, unsigned int block_size) {
    struct device_block *new_dev = malloc(sizeof(struct device_block));

    if (!new_dev)
        return NULL;

    mutex_init(&new_dev->mtx);
    mutex_lock(&new_dev->mtx);

    new_dev->driver = driver;
    new_dev->block_size = block_size;

    return new_dev;
}

int device_char_open(struct device_char *dev, int flags) {
    return dev->driver->open(dev, flags);
}

int device_char_close(struct device_char *dev) {
    return dev->driver->close(dev);
}

ssize_t device_char_read(struct device_char *dev, char *buf, size_t count, unsigned long pos) {
    return dev->driver->read(dev, buf, count, pos);
}

ssize_t device_char_write(struct device_char *dev, const char *buf, size_t count, unsigned long pos) {
    return dev->driver->write(dev, buf, count, pos);
}

int device_block_open(struct device_block *dev, int flags) {
    return dev->driver->open(dev, flags);
}

int device_block_close(struct device_block *dev) {
    return dev->driver->close(dev);
}

ssize_t device_block_read(struct device_block *dev, char *buf, unsigned int block_count, unsigned long pos) {
    return dev->driver->read(dev, buf, block_count, pos);
}

ssize_t device_block_write(struct device_block *dev, const char *buf, unsigned int block_count, unsigned long pos) {
    return dev->driver->write(dev, buf, block_count, pos);
}

int dummy_char_open(struct device_char *dev, int flags) {
    (void) dev; (void) flags;
    return 0;
}

int dummy_char_close(struct device_char *dev) {
    (void) dev;
    return 0;
}

ssize_t dummy_char_read(struct device_char *dev, char *buf, size_t count, unsigned long pos) {
    (void) dev; (void) buf; (void) count; (void) pos;
    return 0;
}

ssize_t dummy_char_write(struct device_char *dev, const char *buf, size_t count, unsigned long pos) {
    (void) dev; (void) buf; (void) count; (void) pos;
    return 0;
}

int dummy_block_open(struct device_block *dev, int flags) {
    (void) dev; (void) flags;
    return 0;
}

int dummy_block_close(struct device_block *dev) {
    (void) dev;
    return 0;
}

ssize_t dummy_block_read(struct device_block *dev, char *buf, unsigned int block_count, unsigned long pos) {
    (void) dev; (void) buf; (void) block_count; (void) pos;
    return 0;
}

ssize_t dummy_block_write(struct device_block *dev, const char *buf, unsigned int block_count, unsigned long pos) {
    (void) dev; (void) buf; (void) block_count; (void) pos;
    return 0;
}
