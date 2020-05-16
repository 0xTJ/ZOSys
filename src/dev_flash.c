#include "device.h"
#include "file.h"
#include "mem.h"
#include "module.h"
#include <stdlib.h>
#include <stdint.h>

int dev_flash_init(void);
void dev_flash_exit(void);

struct module dev_flash_module = {
    dev_flash_init,
    dev_flash_exit
};

enum flash_chip {
    SST39SF010A,
    SST39SF020A,
    SST39SF040A,
};

struct flash_chip_desc {
    unsigned long chip_size;
    unsigned long sector_size;
};

struct flash_chip_desc flash_chip_descs[] = {
    [SST39SF010A] = {
        131072, 4096
    },
    [SST39SF020A] = {
        262144, 4096
    },
    [SST39SF040A] = {
        524288, 4096
    },
};

struct flash_instance {
    enum flash_chip chip;
    uint32_t base_addr;
};

struct flash_instance flash_instance_0 = {
    SST39SF040A,
    0
};

struct file_ops flash_driver = {
    .read = flash_read,
    .write = flash_write,
    .ioctl = flash_ioctl,
};

int dev_flash_init(void) {
    device_register_driver(4, &flash_driver);

    return 0;
}

void dev_flash_exit(void) {
    // TODO: Unregister driver

    return;
}

ssize_t flash_read(struct file *file_ptr, char *buf, size_t count, unsigned long pos) {
    ssize_t result = -1;
    struct flash_instance *instance = NULL;

    if (file_ptr->special.minor == 0) {
        instance = &flash_instance_0;
    } else {
        // Not flash0
    }

    if (!instance) {
        return -1;
    }
    if (pos >= flash_chip_descs[instance->chip].chip_size) {
        return -1;
    }
    unsigned long size_left = flash_chip_descs[instance->chip].chip_size - pos;
    if (count > size_left) {
        count = size_left;
    }

    mem_memcpy_kernel_from_long(buf, instance->base_addr + pos, count);
    result = count;

    return result;
}

static void sst39sfxxxa_wait(void) {
    char tmp[2];
    do {
        mem_memcpy_kernel_from_long(&tmp, 0, 2);
    } while ((tmp[0] ^ tmp[1]) & (1 << 6));
}

ssize_t flash_write_sst39sfxxxa(struct flash_instance *instance, const char *buf, size_t count, unsigned long pos) {
    unsigned long flash_base = instance->base_addr;
    for (unsigned long bytes_written = 0; bytes_written < count; ++bytes_written) {
        char tmp;
        tmp = 0xAA;
        mem_memcpy_long_from_kernel(flash_base + 0x5555, &tmp, 1);
        tmp = 0x55;
        mem_memcpy_long_from_kernel(flash_base + 0x2AAA, &tmp, 1);
        tmp = 0xA0;
        mem_memcpy_long_from_kernel(flash_base + 0x5555, &tmp, 1);
        mem_memcpy_long_from_kernel(flash_base + pos + bytes_written, &buf[bytes_written], 1);
        sst39sfxxxa_wait();
    }
    return count;
}

ssize_t flash_write(struct file *file_ptr, const char *buf, size_t count, unsigned long pos) {
    int result = -1;
    struct flash_instance *instance = NULL;

    if (file_ptr->special.minor == 0) {
        instance = &flash_instance_0;
    } else {
        // Not flash0
    }

    if (!instance) {
        return -1;
    }
    if (pos >= flash_chip_descs[instance->chip].chip_size) {
        return -1;
    }
    unsigned long size_left = flash_chip_descs[instance->chip].chip_size - pos;
    if (count > size_left) {
        return -1;
    }

    switch (instance->chip) {
    case SST39SF010A:
    case SST39SF020A:
    case SST39SF040A:
        result = flash_write_sst39sfxxxa(instance, buf, count, pos);
    }

    return result;
}

// TODO: Make this file thread-safe

int flash_ioctl_sst39sfxxxa(struct flash_instance *instance, int request, uintptr_t argp) {
    unsigned long flash_base = instance->base_addr;
    switch (request) {
    case 0: {
            unsigned char tmp;
            tmp = 0xAA;
            mem_memcpy_long_from_kernel(flash_base + 0x5555, &tmp, 1);
            tmp = 0x55;
            mem_memcpy_long_from_kernel(flash_base + 0x2AAA, &tmp, 1);
            tmp = 0x80;
            mem_memcpy_long_from_kernel(flash_base + 0x5555, &tmp, 1);
            tmp = 0xAA;
            mem_memcpy_long_from_kernel(flash_base + 0x5555, &tmp, 1);
            tmp = 0x55;
            mem_memcpy_long_from_kernel(flash_base + 0x2AAA, &tmp, 1);
            tmp = 0x30;
            mem_memcpy_long_from_kernel(flash_base + (unsigned long) argp * flash_chip_descs[instance->chip].sector_size, &tmp, 1);
            sst39sfxxxa_wait();
        }
        break;
    case 1: {
            unsigned char tmp;
            tmp = 0xAA;
            mem_memcpy_long_from_kernel(flash_base + 0x5555, &tmp, 1);
            tmp = 0x55;
            mem_memcpy_long_from_kernel(flash_base + 0x2AAA, &tmp, 1);
            tmp = 0x80;
            mem_memcpy_long_from_kernel(flash_base + 0x5555, &tmp, 1);
            tmp = 0xAA;
            mem_memcpy_long_from_kernel(flash_base + 0x5555, &tmp, 1);
            tmp = 0x55;
            mem_memcpy_long_from_kernel(flash_base + 0x2AAA, &tmp, 1);
            tmp = 0x10;
            mem_memcpy_long_from_kernel(flash_base + 0x5555, &tmp, 1);
            sst39sfxxxa_wait();
        }
        break;
    }
    return 0;
}

int flash_ioctl(struct file *file_ptr, int request, uintptr_t argp) {
    int result = -1;
    struct flash_instance *instance = NULL;

    if (file_ptr->special.minor == 0) {
        instance = &flash_instance_0;
    } else {
        // Not flash0
    }

    if (!instance) {
        return -1;
    }

    switch (instance->chip) {
    case SST39SF010A:
    case SST39SF020A:
    case SST39SF040A:
        result = flash_ioctl_sst39sfxxxa(instance, request, argp);
        break;
    }

    return result;
}
