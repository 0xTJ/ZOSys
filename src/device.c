#include "device.h"
#include <adt.h>
#include <stdlib.h>

struct device *device_char_new(struct device_char_driver *driver) {
    struct device *new_dev = malloc(sizeof(struct device));

    if (!new_dev)
        return NULL;

    mutex_init(&new_dev->mtx);
    mutex_lock(&new_dev->mtx);

    new_dev->type = DEVICE_CHARACTER;
    new_dev->character.driver = driver;

    return new_dev;
}
struct device *device_block_new(struct device_block_driver *driver, unsigned int block_size) {
    struct device *new_dev = malloc(sizeof(struct device));

    if (!new_dev)
        return NULL;

    mutex_init(&new_dev->mtx);
    mutex_lock(&new_dev->mtx);

    new_dev->type = DEVICE_BLOCK;
    new_dev->block.driver = driver;
    new_dev->block.block_size = block_size;

    return new_dev;
}

int device_open(struct device *dev, int flags) {
    if (dev->type == DEVICE_CHARACTER) {
        return dev->character.driver->open(dev, flags);
    } else if (dev->type == DEVICE_BLOCK) {
        return dev->block.driver->open(dev, flags);
    } else {
        return -1;
    }
}

int device_close(struct device *dev) {
    if (dev->type == DEVICE_CHARACTER) {
        return dev->character.driver->close(dev);
    } else if (dev->type == DEVICE_BLOCK) {
        return dev->block.driver->close(dev);
    } else {
        return -1;
    }
}

ssize_t device_char_read(struct device *dev, char *buf, size_t count, unsigned long pos) {
    if (dev->type == DEVICE_CHARACTER) {
        return dev->character.driver->read(dev, buf, count, pos);
    } else {
        return -1;
    }
}

ssize_t device_char_write(struct device *dev, const char *buf, size_t count, unsigned long pos) {
    if (dev->type == DEVICE_CHARACTER) {
        return dev->character.driver->write(dev, buf, count, pos);
    } else {
        return -1;
    }
}

ssize_t device_block_read(struct device *dev, char *buf, unsigned int block_count, unsigned long pos) {
    if (dev->type == DEVICE_BLOCK) {
        return dev->block.driver->read(dev, buf, block_count, pos);
    } else {
        return -1;
    }
}

ssize_t device_block_write(struct device *dev, const char *buf, unsigned int block_count, unsigned long pos) {
    if (dev->type == DEVICE_BLOCK) {
        return dev->block.driver->write(dev, buf, block_count, pos);
    } else {
        return -1;
    }
}

int dummy_open(struct device *dev, int flags) {
    (void) dev; (void) flags;
    return 0;
}

int dummy_close(struct device *dev) {
    (void) dev;
    return 0;
}

ssize_t dummy_char_read(struct device *dev, char *buf, size_t count, unsigned long pos) {
    (void) dev; (void) buf; (void) count; (void) pos;
    return 0;
}

ssize_t dummy_char_write(struct device *dev, const char *buf, size_t count, unsigned long pos) {
    (void) dev; (void) buf; (void) count; (void) pos;
    return 0;
}

ssize_t dummy_block_read(struct device *dev, char *buf, unsigned int block_count, unsigned long pos) {
    (void) dev; (void) buf; (void) block_count; (void) pos;
    return 0;
}

ssize_t dummy_block_write(struct device *dev, const char *buf, unsigned int block_count, unsigned long pos) {
    (void) dev; (void) buf; (void) block_count; (void) pos;
    return 0;
}
