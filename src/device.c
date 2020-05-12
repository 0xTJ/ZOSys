#include "device.h"
#include <adt.h>
#include <stdlib.h>

struct file_ops *device_drivers[DEVICE_MAJOR_NUM] = {0};

int device_register_driver(int major, struct file_ops *drv) __critical {
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
    struct file_ops *driver = device_drivers[file_ptr->special.major - 1];
    if (!driver) {
        return -1;
    }
    if (!driver->open) {
        return 0;
    }
    return driver->open(file_ptr, flags);
}

int device_close(struct file *file_ptr) {
    struct file_ops *driver = device_drivers[file_ptr->special.major - 1];
    if (!driver) {
        return -1;
    }
    if (!driver->close) {
        return 0;
    }
    return driver->close(file_ptr);
}

ssize_t device_read(struct file *file_ptr, char *buf, size_t count, unsigned long pos) {
    struct file_ops *driver = device_drivers[file_ptr->special.major - 1];
    if (!driver) {
        return -1;
    }
    if (!driver->read) {
        return -1;
    }
    return driver->read(file_ptr, buf, count, pos);
}

ssize_t device_write(struct file *file_ptr, const char *buf, size_t count, unsigned long pos) {
    struct file_ops *driver = device_drivers[file_ptr->special.major - 1];
    if (!driver) {
        return -1;
    }
    if (!driver->write) {
        return -1;
    }
    return driver->write(file_ptr, buf, count, pos);
}

int device_ioctl(struct file *file_ptr, int request, uintptr_t argp) {
    struct file_ops *driver = device_drivers[file_ptr->special.major - 1];
    if (!driver) {
        return -1;
    }
    if (!driver->ioctl) {
        return -1;
    }
    return driver->ioctl(file_ptr, request, argp);
}
