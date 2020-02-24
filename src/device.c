#include "device.h"
#include <adt.h>
#include <stdlib.h>

struct device_driver *device_drivers[DEVICE_MAJOR_NUM] = {0};

int device_register_driver(int major, struct device_driver *drv) __critical {
    if (major == 0) {
        for (int new_major = 1; new_major <= DEVICE_MAJOR_NUM; ++new_major) {
            if (!device_drivers[new_major - 1]) {
                device_drivers[new_major - 1] = drv;
                return new_major;
            }
        }
        return -1;
    } else if (major > 0 && major <= DEVICE_MAJOR_NUM) {
        if (device_drivers[major - 1]) {
            // Major number already in use
            return -1;
        } else {
            device_drivers[major - 1] = drv;
            return 0;
        }
    } else {
        return -1;
    }
}

int device_open(struct file *file_ptr, int flags) {
    struct device_driver *driver = device_drivers[file_ptr->special.major - 1];
    if (!driver || !driver->open) {
        return -1;
    }
    return driver->open(file_ptr, flags);
}

int device_close(struct file *file_ptr) {
    struct device_driver *driver = device_drivers[file_ptr->special.major - 1];
    if (!driver || !driver->close) {
        return -1;
    }
    return driver->close(file_ptr);
}

ssize_t device_read(struct file *file_ptr, char *buf, size_t count, unsigned long pos) {
    struct device_driver *driver = device_drivers[file_ptr->special.major - 1];
    if (!driver || !driver->read) {
        return -1;
    }
    return driver->read(file_ptr, buf, count, pos);
}

ssize_t device_write(struct file *file_ptr, const char *buf, size_t count, unsigned long pos) {
    struct device_driver *driver = device_drivers[file_ptr->special.major - 1];
    if (!driver || !driver->write) {
        return -1;
    }
    return driver->write(file_ptr, buf, count, pos);
}

int dev_dummy_open(struct file *file_ptr, int flags) {
    (void) file_ptr; (void) flags;
    return 0;
}

int dev_dummy_close(struct file *file_ptr) {
    (void) file_ptr;
    return 0;
}

ssize_t dev_dummy_read(struct file *file_ptr, char *buf, size_t count, unsigned long pos) {
    (void) file_ptr; (void) buf; (void) count; (void) pos;
    return -1;
}

ssize_t dev_dummy_write(struct file *file_ptr, const char *buf, size_t count, unsigned long pos) {
    (void) file_ptr; (void) buf; (void) count; (void) pos;
    return-1;
}
