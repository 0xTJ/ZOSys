#include "device.h"
#include "file.h"
#include "module.h"
#include "circular_buffer.h"
#include <cpu.h>
#include <intrinsic.h>
#include <stdlib.h>
#include <string.h>

int dev_sermem_init(void);
void dev_sermem_exit(void);

int sermem_perform_seek(struct file *backing, unsigned long pos);
ssize_t sermem_perform_read(struct file *backing, char *buf, size_t count, unsigned long pos);
ssize_t sermem_perform_write(struct file *backing, const char *buf, size_t count, unsigned long pos);

struct module dev_sermem_module = {
    dev_sermem_init,
    dev_sermem_exit
};

struct file_ops sermem_driver = {
    .read = sermem_read,
    .write = sermem_write,
};

int dev_sermem_init(void) {
    device_register_driver(2, &sermem_driver);

    return 0;
}

void dev_sermem_exit(void) {
    // TODO: Unregister driver

    return;
}

int sermem_perform_seek(struct file *backing, unsigned long pos) {
    file_write(backing, "s", 1, 0); // TODO: Check for error

    char pos_str[sizeof(pos) * 2 + 1];
    ltoa(pos, pos_str, 16);
    file_write(backing, pos_str, strlen(pos_str), 0); // TODO: Check for error

    return 0;
}

ssize_t sermem_perform_read(struct file *backing, char *buf, size_t count, unsigned long pos) {
    sermem_perform_seek(backing, pos); // TODO: Check for error

    size_t result;
    for (result = 0; result < count; ++result) {
        file_write(backing, "r", 1, 0); // TODO: Check for error

        char temp[3];
        file_read(backing, temp, 2, 0); // TODO: Check for error
        temp[2] = '\0';
        *(buf++) = atoi(temp);
    }

    return result;
}

ssize_t sermem_perform_write(struct file *backing, const char *buf, size_t count, unsigned long pos) {
    sermem_perform_seek(backing, pos); // TODO: Check for error

    size_t result;
    for (result = 0; result < count; ++result) {
        file_write(backing, "w", 1, 0); // TODO: Check for error

        char temp[3];
        itoa(*(buf++), temp, 16);
        if (temp[1] == '\0') {
            temp[2] = '\0';
            temp[1] = temp[0];
            temp[0] = '0';
        }
        file_write(backing, temp, 2, 0); // TODO: Check for error
    }

    return result;
}

ssize_t sermem_read(struct file *file_ptr, char *buf, size_t count, unsigned long pos) {
    ssize_t result = -1;

    if (file_ptr->special.minor == 0) {
        if (file_ptr->special.backing) {
            result = sermem_perform_read(file_ptr->special.backing, buf, count, pos);
        }
    } else {
        // Not sermem0
    }

    return result;
}

ssize_t sermem_write(struct file *file_ptr, const char *buf, size_t count, unsigned long pos) {
    ssize_t result = -1;

    if (file_ptr->special.minor == 0) {
        if (file_ptr->special.backing) {
            result = sermem_perform_write(file_ptr->special.backing, buf, count, pos);
        }
    } else {
        // Not sermem0
    }

    return result;
}
