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

    return result;
}

ssize_t flash_write(struct file *file_ptr, const char *buf, size_t count, unsigned long pos) {
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
        return -1;
    }
    if (pos % flash_chip_descs[instance->chip].sector_size) {
        return -1;
    }
    if (count % flash_chip_descs[instance->chip].sector_size) {
        return -1;
    }

    return -1;
}
