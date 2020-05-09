#include "file.h"
#include "device.h"
#include "panic.h"
#include "process.h"
#include "vfs.h"
#include <errno.h>
#include <fcntl.h>
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
        panic();
    }
    ptr->ref_count += 1;
}

void file_file_unref(struct file *ptr) __critical {
    if (ptr->ref_count == 0) {
        panic();
    }
    ptr->ref_count -= 1;
    if (ptr->ref_count == 0) {
        file_file_free(ptr);
    }
}

void file_init_plain(struct file *file_ptr, struct mountpoint *mp, ino_t inode) {
    file_ptr->type = FILE_PLAIN;
    file_ptr->plain.mp = mp;
    file_ptr->plain.inode = inode;
}

void file_init_special(struct file *file_ptr, int major, int minor, struct file *backing) {
    file_ptr->type = FILE_SPECIAL;
    file_ptr->special.major = major;
    file_ptr->special.minor = minor;
    if (backing) {
        file_file_ref(backing);
    }
    file_ptr->special.backing = backing;
}

void file_init_directory(struct file *file_ptr, struct mountpoint *mp, ino_t inode) {
    file_ptr->type = FILE_DIRECTORY;
    file_ptr->directory.mp = mp;
    file_ptr->directory.inode = inode;
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
    struct mountpoint *mp = NULL;
    struct file *file_ptr = NULL;
    int result = -1;
    const char *pathname_in_mount = pathname;

    if (pathname[0] == '\0' || pathname[1] != ':') {
        // Relative path
        if (current_proc->cwd) {
            mp = current_proc->cwd->directory.mp;
        } else {
            // No CWD exists
            panic();
            return NULL;
        }
    } else {
        // Absolute path
        mp = vfs_get_mount(pathname);
        pathname_in_mount += 2;
    }

    if (mp) {
        file_ptr = mp->fs->get_file(mp, pathname_in_mount);
    }

    if (file_ptr) {
        switch (file_ptr->type) {
        case FILE_PLAIN:
            if (mp->fs->ops && mp->fs->ops->open) {
                result = mp->fs->ops->open(file_ptr, flags);
            } else {
                result = 0; 
            }
            break;
        case FILE_SPECIAL:
            result = device_open(file_ptr, flags);
            break;
        case FILE_DIRECTORY:
            if (mp->fs->ops && mp->fs->ops->open) {
                result = mp->fs->ops->open(file_ptr, flags);
            } else {
                result = 0; 
            }
            break;
        default:
            panic();
            result = -1;
            break;
        }
    }

    if (result < 0)
        file_ptr = NULL;

    return file_ptr;
}

int file_close(struct file *file_ptr) {
    switch (file_ptr->type) {
    case FILE_PLAIN:
        if (file_ptr->plain.mp->fs->ops && file_ptr->plain.mp->fs->ops->close) {
            return file_ptr->plain.mp->fs->ops->close(file_ptr);
        } else {
            return 0; 
        }
    case FILE_SPECIAL:
        return device_close(file_ptr);
    case FILE_DIRECTORY:
        return -1;
    default:
        panic();
        return -1;
    }
}

ssize_t file_read(struct file *file_ptr, char *buf, size_t count, unsigned long pos) {
    switch (file_ptr->type) {
    case FILE_PLAIN:
        if (file_ptr->plain.mp->fs->ops && file_ptr->plain.mp->fs->ops->read) {
            return file_ptr->plain.mp->fs->ops->read(file_ptr, buf, count, pos);
        } else {
            return -1; 
        }
    case FILE_SPECIAL:
        return device_read(file_ptr, buf, count, pos);
    case FILE_DIRECTORY:
        return -1;
    default:
        panic();
        return -1;
    }
}

ssize_t file_write(struct file *file_ptr, const char *buf, size_t count, unsigned long pos) {
    switch (file_ptr->type) {
    case FILE_PLAIN:
        if (file_ptr->plain.mp->fs->ops && file_ptr->plain.mp->fs->ops->write) {
            return file_ptr->plain.mp->fs->ops->write(file_ptr, buf, count, pos);
        } else {
            return 0; 
        }
    case FILE_SPECIAL:
        return device_write(file_ptr, buf, count, pos);
    case FILE_DIRECTORY:
        return -1;
    default:
        panic();
        return -1;
    }
}

int file_readdirent(struct file *file_ptr, struct dirent *dirp, unsigned int count) {
    switch (file_ptr->type) {
    case FILE_DIRECTORY:
        if (file_ptr->plain.mp->fs->ops && file_ptr->plain.mp->fs->ops->readdirent) {
            return file_ptr->plain.mp->fs->ops->readdirent(file_ptr, dirp, count);
        } else {
            return -1; 
        }
    case FILE_PLAIN:
    case FILE_SPECIAL:
        return -1;
    default:
        panic();
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
        file_open_file_free(open_file);
        return -1;
    }

    open_file->pos = 0;
    current_proc->open_files[found_fd] = open_file;
    return found_fd;
}

int sys_close(int fd) {
    if (fd < 0 || fd >= MAX_OPEN_FILES)
        return -1;

    struct open_file *open_file = current_proc->open_files[fd];
    if (!open_file)
        return -1;
    file_close(open_file->file);
    file_open_file_free(open_file);
    current_proc->open_files[fd] = NULL;
    return 0;
}

ssize_t sys_read(int fd, USER_PTR(char) buf, size_t count) {
    struct open_file *open_file = current_proc->open_files[fd];
    if (!open_file)
        return -1;

    // Check bounds
    if (buf < 0x1000 || (unsigned long) buf + count > 0xF000)
        return -1;

    ssize_t done_count = 0;
    while (done_count < count) {
        size_t inc_count = count - done_count;
        if (inc_count > MEM_USER_BUFFER_SIZE)
            inc_count = MEM_USER_BUFFER_SIZE;
        if (inc_count == 0)
            break;
        
        char *buf_copy = mem_get_user_buffer();
        ssize_t this_count = file_read(open_file->file, buf_copy, inc_count, open_file->pos);

        if (this_count < 0) {
            if (done_count == 0)
                done_count = -1;
            break;
        }

        mem_copy_from_user_buffer(buf + done_count, this_count);

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
            break;
        
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

int sys_readdirent(unsigned int fd, USER_PTR(struct dirent) dirp, unsigned int count) {
    struct open_file *open_file = current_proc->open_files[fd];
    if (!open_file)
        return -1;

    // Check bounds
    if (dirp < 0x1000 || (unsigned long) dirp + sizeof(struct dirent) > 0xF000)
        return -1;
        
    char *buf_copy = mem_get_user_buffer();
    int result = file_readdirent(open_file->file, (struct dirent *) buf_copy, count);
    mem_copy_from_user_buffer(dirp, sizeof(struct dirent));

    return result;
}

int sys_chdir(USER_PTR(const char) path) {
    char *pathname_copied = mem_copy_to_user_buffer(path, MEM_USER_BUFFER_SIZE);
    pathname_copied[MEM_USER_BUFFER_SIZE] = '\0';

    struct file *opened_dir = file_open(pathname_copied, O_RDONLY);
    if (!opened_dir || opened_dir->type != FILE_DIRECTORY) {
        file_file_unref(opened_dir);
        return -1;
    }

    file_file_unref(current_proc->cwd);
    current_proc->cwd = opened_dir;

    return 0;
}

int sys_fchdir(int fildes) {
    if (fildes < 0 || fildes >= MAX_OPEN_FILES)
        return -1;

    struct open_file *opened_dir = current_proc->open_files[fildes];
    if (!opened_dir || opened_dir->file->type != FILE_DIRECTORY) {
        return -1;
    }

    file_file_unref(current_proc->cwd);
    file_file_ref(opened_dir->file);
    current_proc->cwd = opened_dir->file;

    return 0;
}
