#include "file.h"
#include "process.h"
#include "vfs.h"
#include <stdlib.h>
#include <string.h>

struct file *file_file_new(void) {
    struct file *result = malloc(sizeof(struct file));
    if (result) {
        memset(result, 0, sizeof(result));
        file_file_ref(result);
    }
    return result;
}

void file_file_free(struct file *ptr) {
    // Safe to free NULL
    free(ptr);
}

void file_file_ref(struct file *ptr) __critical {
    if (ptr->ref_count == SIZE_MAX) {
        // panic()
        // TODO: Add panic
    }
    ptr->ref_count += 1;
}

void file_file_unref(struct file *ptr) __critical {
    if (ptr->ref_count == 0) {
        // panic()
        // TODO: Add panic
    }
    ptr->ref_count -= 1;
    if (ptr->ref_count == 0) {
        file_file_free(ptr);
    }
}

struct open_file *file_open_file_new(void) {
    struct open_file *result = malloc(sizeof(struct open_file));
    memset(result, 0, sizeof(result));
    return result;
}

struct open_file *file_open_file_clone(struct open_file *src) {
    struct open_file *dest = NULL;
    if (src) {
        dest = malloc(sizeof(struct open_file));
        if (dest) {
            file_file_ref(dest->file);
            memcpy(dest, src, sizeof(dest));
        }
    }
    return dest;
}

void file_open_file_free(struct open_file *ptr) {
    if (ptr) {
        if (ptr->file) {
            file_file_unref(ptr->file);
        }
        free(ptr);
    }
}

struct file *file_open(const char *pathname, int flags) {
    struct filesystem *fs = NULL;
    struct file *file = NULL;
    int result = -1;

    fs = vfs_get_fs(pathname);

    if (fs) {
        file = fs->get_file(fs, pathname + 2);
    }

    if (file) {
        switch (file->type) {
        case FILE_CHAR_DEV:
            result = device_char_open(file->dev_char, flags);
            break;
        case FILE_BLOCK_DEV:
            result = device_block_open(file->dev_block, flags);
            break;
        default:
            result = -1;
            break;
        }
    }

    if (result < 0)
        file = NULL;

    return file;
}

int file_close(struct file *file) {
    switch (file->type) {
    case FILE_CHAR_DEV:
        return device_char_close(file->dev_char);
    case FILE_BLOCK_DEV:
        return device_block_close(file->dev_block);
    default:
        return -1;
    }
}

ssize_t file_read(struct file *file, char *buf, size_t count, unsigned long pos) {
    switch (file->type) {
    case FILE_CHAR_DEV:
        return device_char_read(file->dev_char, buf, count, pos);
    case FILE_BLOCK_DEV:
        return device_block_read(file->dev_block, buf, count, pos);
    default:
        return -1;
    }
}

ssize_t file_write(struct file *file, const char *buf, size_t count, unsigned long pos) {
    switch (file->type) {
    case FILE_CHAR_DEV:
        return device_char_write(file->dev_char, buf, count, pos);
    case FILE_BLOCK_DEV:
        return device_block_write(file->dev_block, buf, count, pos);
    default:
        return -1;
    }
}

int sys_open(USER_PTR(char) pathname, int flags) {
    int found_fd = -1;
    for (found_fd = 0; found_fd < MAX_OPEN_FILES; ++found_fd)
        if (!current_proc->open_files[found_fd])
            break;
    if (found_fd == MAX_OPEN_FILES)
        return -1;

    struct open_file *open_file = file_open_file_new();
    if (!open_file)
        return -1;

    char *pathname_copied = mem_copy_to_user_buffer(pathname, MEM_USER_BUFFER_SIZE);
    pathname_copied[MEM_USER_BUFFER_SIZE] = '\0';

    open_file->file = file_open(pathname_copied, flags);
    if (!open_file->file) {
        free(open_file);
        return -1;
    }

    open_file->pos = 0;
    current_proc->open_files[found_fd] = open_file;
    return found_fd;
}

int sys_close(int fd) {
    struct open_file *open_file = current_proc->open_files[fd];
    if (!open_file)
        return -1;
    file_close(open_file->file);
    free(open_file);
    current_proc->open_files[fd] = NULL;
    return 0;
}

ssize_t sys_read(int fd, USER_PTR(char) buf, size_t count) {
    struct open_file *open_file = current_proc->open_files[fd];
    if (!open_file)
        return -1;

    // Check bounds
    if (buf < 0x1000 || (unsigned long) buf + count >= 0xF000)
        return -1;

    ssize_t done_count = 0;
    while (done_count < count) {
        size_t inc_count = count - done_count;
        if (inc_count > MEM_USER_BUFFER_SIZE)
            inc_count = MEM_USER_BUFFER_SIZE;
        if (inc_count == 0)
            continue;
        
        char *buf_copy = mem_get_user_buffer();
        ssize_t this_count = file_read(open_file->file, buf_copy, count, open_file->pos);

        if (this_count < 0) {
            if (done_count == 0)
                done_count = -1;
            break;
        }

        mem_copy_from_user_buffer(buf + done_count, count);
        done_count += this_count;
        open_file->pos += this_count;
        if (this_count < inc_count)
            break;
    }

    return done_count;
}

ssize_t sys_write(int fd, USER_PTR(char) buf, size_t count) {
    struct open_file *open_file = current_proc->open_files[fd];
    if (!open_file)
        return -1;

    // Check bounds
    if (buf < 0x1000 || (unsigned long) buf + count >= 0xF000)
        return -1;

    ssize_t done_count = 0;
    while (done_count < count) {
        size_t inc_count = count - done_count;
        if (inc_count > MEM_USER_BUFFER_SIZE)
            inc_count = MEM_USER_BUFFER_SIZE;
        if (inc_count == 0)
            continue;
        
        char *buf_copy = mem_copy_to_user_buffer(buf + done_count, inc_count);
        ssize_t this_count = file_write(open_file->file, buf_copy, inc_count, open_file->pos);

        if (this_count < 0) {
            if (done_count == 0)
                done_count = -1;
            break;
        }

        done_count += this_count;
        open_file->pos += this_count;
        if (this_count < inc_count)
            break;
    }

    return done_count;
}
