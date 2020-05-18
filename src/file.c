#include "file.h"
#include "device.h"
#include "mem.h"
#include "panic.h"
#include "process.h"
#include "vfs.h"
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "kio.h"

struct file *file_file_new(void) {
    struct file *result = malloc(sizeof(struct file));
    if (result) {
        memset(result, 0, sizeof(struct file));
        file_file_ref(result);
    }
    return result;
}

void file_file_free(struct file *ptr) {
    // Safe to free NULL
    free(ptr);
}

void file_file_ref(struct file *ptr) __critical {
    if (!ptr) {
        panic();
    }
    if (ptr->ref_count == SIZE_MAX) {
        panic();
    }
    ptr->ref_count += 1;
}

void file_file_unref(struct file *ptr) __critical {
    if (!ptr) {
        panic();
    }
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

void file_init_pipe(struct file *file_ptr_read, struct file *file_ptr_write, struct circular_buffer *circ_buf) {
    file_ptr_read->type = FILE_PIPE;
    file_ptr_write->type = FILE_PIPE;
    file_ptr_read->pipe.end = PIPE_READ;
    file_ptr_write->pipe.end = PIPE_WRITE;
    file_ptr_read->pipe.circ_buf = circ_buf;
    file_ptr_write->pipe.circ_buf = circ_buf;
}

struct open_file *file_open_file_new(void) {
    struct open_file *result = malloc(sizeof(struct open_file));
    if (result) {
        memset(result, 0, sizeof(struct open_file));
        file_open_file_ref(result);
    }
    return result;
}

struct open_file *file_open_file_clone(struct open_file *src) {
    struct open_file *dest = NULL;
    if (src) {
        dest = malloc(sizeof(struct open_file));
        if (dest) {
            file_file_ref(src->file);
            memcpy(dest, src, sizeof(dest));
            dest->ref_count = 0;
            file_open_file_ref(dest);
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

void file_open_file_ref(struct open_file *ptr) __critical {
    if (!ptr) {
        panic();
    }
    if (ptr->ref_count == SIZE_MAX) {
        panic();
    }
    ptr->ref_count += 1;
}

void file_open_file_unref(struct open_file *ptr) __critical {
    if (!ptr) {
        panic();
    }
    if (ptr->ref_count == 0) {
        panic();
    }
    ptr->ref_count -= 1;
    if (ptr->ref_count == 0) {
        file_open_file_free(ptr);
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
    case FILE_PIPE:
        return 0;
    default:
        panic();
        return -1;
    }
}

static ssize_t pipe_read(struct file *file_ptr, char *buf, size_t count) __critical {
    ssize_t count_done;
    for (count_done = 0; count_done < count; ++count_done) {
        int tmp = circular_buffer_get(file_ptr->pipe.circ_buf);
        if (tmp == -1) {
            break;
        }
        buf[count_done] = tmp;
    }
    return count_done;
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
    case FILE_PIPE:
        if (file_ptr->pipe.end != PIPE_READ) {
            return -1;
        }
        return pipe_read(file_ptr, buf, count);
    default:
        panic();
        return -1;
    }
}

static ssize_t pipe_write(struct file *file_ptr, const char *buf, size_t count) __critical {
    ssize_t count_done;
    for (count_done = 0; count_done < count; ++count_done) {
        if (circular_buffer_is_full(file_ptr->pipe.circ_buf)) {
            break;
        }
        circular_buffer_put(file_ptr->pipe.circ_buf, buf[count_done]);
    }
    return count_done;
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
    case FILE_PIPE:
        if (file_ptr->pipe.end != PIPE_WRITE) {
            return -1;
        }
        return pipe_write(file_ptr, buf, count);
    default:
        panic();
        return -1;
    }
}

int file_ioctl(struct file *file_ptr, int request, uintptr_t argp) {
    switch (file_ptr->type) {
    case FILE_DIRECTORY:
    case FILE_PLAIN:
        return -1;
    case FILE_SPECIAL:
        return device_ioctl(file_ptr, request, argp);
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
    file_open_file_unref(open_file);
    current_proc->open_files[fd] = NULL;
    return 0;
}

ssize_t sys_read(int fd, USER_PTR(char) buf, size_t count) {
    if (fd < 0 || fd >= MAX_OPEN_FILES)
        return -1;

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
    if (fd < 0 || fd >= MAX_OPEN_FILES)
        return -1;

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

int sys_ioctl(int fd, int request, USER_PTR(char) argp) {
    if (fd < 0 || fd >= MAX_OPEN_FILES)
        return -1;

    struct open_file *open_file = current_proc->open_files[fd];
    if (!open_file)
        return -1;

    int result = file_ioctl(open_file->file, request, (uintptr_t) argp);

    return result;
}

off_t sys_lseek(int fd, off_t offset, int whence) {
    if (fd < 0 || fd >= MAX_OPEN_FILES)
        return -1;

    kio_puts("sys_lseek(");
    kio_put_ui(fd);
    kio_puts(", ");
    kio_put_ul(offset);
    kio_puts(", ");
    kio_put_ui(whence);
    kio_puts(");");

    struct open_file *open_file = current_proc->open_files[fd];
    if (!open_file)
        return -1;

    switch (whence) {
    case SEEK_SET:
        open_file->pos = offset;
        break;
    case SEEK_CUR:
        open_file->pos += offset;
        break;
    case SEEK_END:
        return -1;
    default:
        return -1;
    }

    off_t result = open_file->pos;

    return result;
}

int sys_readdirent(int fd, USER_PTR(struct dirent) dirp, unsigned int count) {
    if (fd < 0 || fd >= MAX_OPEN_FILES)
        return -1;

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
    if (!opened_dir) {
        return -1;
    } else if (opened_dir->type != FILE_DIRECTORY) {
        file_file_unref(opened_dir);
        return -1;
    }

    if (current_proc->cwd) {
        file_file_unref(current_proc->cwd);
    }
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

    if (current_proc->cwd) {
        file_file_unref(current_proc->cwd);
    }
    file_file_ref(opened_dir->file);
    current_proc->cwd = opened_dir->file;

    return 0;
}

int sys_pipe(USER_PTR(int[2]) pipefd) {
    // Check bounds
    if (pipefd < 0x1000 || (unsigned long) pipefd + sizeof(int [2]) > 0xF000)
        return -1;

    // Allocate file descriptors
    int found_fd_read = -1;
    for (found_fd_read = 0; found_fd_read < MAX_OPEN_FILES; ++found_fd_read)
        if (!current_proc->open_files[found_fd_read])
            break;
    if (found_fd_read == MAX_OPEN_FILES)
        return -1;
    int found_fd_write = -1;
    for (found_fd_write = 0; found_fd_write < MAX_OPEN_FILES; ++found_fd_write)
        if (found_fd_write != found_fd_read && !current_proc->open_files[found_fd_write])
            break;
    if (found_fd_write == MAX_OPEN_FILES)
        return -1;

    // Create new open files
    struct open_file *open_file_read = file_open_file_new();
    if (!open_file_read) {
        return -1;
    }
    struct open_file *open_file_write = file_open_file_new();
    if (!open_file_write) {
        file_open_file_free(open_file_read);
        return -1;
    }

    // Create circular buffer
    struct circular_buffer *circ_buf = malloc(sizeof(struct circular_buffer));
    if (!circ_buf) {
        file_open_file_free(open_file_write);
        file_open_file_free(open_file_read);
        return -1;
    }
    circ_buf->size = 128;
    circ_buf->head = 0;
    circ_buf->tail = 0;
    circ_buf->buffer = malloc(circ_buf->size);
    if (!circ_buf->buffer) {
        free(circ_buf);
        file_open_file_free(open_file_write);
        file_open_file_free(open_file_read);
        return -1;
    }

    // Create new files
    struct file *file_ptr_read = file_file_new();
    if (!file_ptr_read) {
        free(circ_buf->buffer);
        free(circ_buf);
        file_open_file_free(open_file_write);
        file_open_file_free(open_file_read);
        return -1;
    }
    struct file *file_ptr_write = file_file_new();
    if (!file_ptr_write) {
        file_file_free(file_ptr_read);
        free(circ_buf->buffer);
        free(circ_buf);
        file_open_file_free(open_file_write);
        file_open_file_free(open_file_read);
        return -1;
    }

    // Setup pipe
    file_init_pipe(file_ptr_read, file_ptr_write, circ_buf);

    open_file_read->file = file_ptr_read;
    open_file_write->file = file_ptr_write;

    open_file_read->pos = 0;
    open_file_write->pos = 0;
    current_proc->open_files[found_fd_read] = open_file_read;
    current_proc->open_files[found_fd_write] = open_file_write;

    int pipefd_kernel[2] = { found_fd_read, found_fd_write };
    mem_memcpy_user_from_kernel(pipefd, pipefd_kernel, sizeof(int) * 2);

    return 0;
}

int sys_dup2(int oldfd, int newfd) {
    if (oldfd < 0 || oldfd >= MAX_OPEN_FILES)
        return -1;
    if (newfd < 0 || newfd >= MAX_OPEN_FILES)
        return -1;

    struct open_file *old_open_file = current_proc->open_files[oldfd];
    if (!old_open_file)
        return -1;
        
    if (oldfd == newfd)
        return newfd;

    if (current_proc->open_files[newfd]) {
        file_close(current_proc->open_files[newfd]->file);
        file_open_file_unref(current_proc->open_files[newfd]);
        current_proc->open_files[newfd] = NULL;
    }

    file_open_file_ref(current_proc->open_files[oldfd]);
    current_proc->open_files[newfd] = current_proc->open_files[oldfd];

    return newfd;
}
