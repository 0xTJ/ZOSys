#ifndef INCLUDE_FILE_H
#define INCLUDE_FILE_H

#include "mem.h"
#include <stdint.h>
#include <sys/types.h>

#define MAX_OPEN_FILES 8

enum file_type {
    FILE_PLAIN,
    FILE_SPECIAL,
    FILE_DIRECTORY,
};

struct file {
    enum file_type type;
    size_t ref_count;
    union {
        struct {
            struct mountpoint *mp;
            ino_t inode;
        } plain;
        struct {
            int major : 8;
            int minor : 8;
            struct file *backing;
        } special;
        struct {
            struct mountpoint *mp;
            ino_t inode;
        } directory;
    };
};

struct dirent {
    ino_t d_ino;
    char d_name[256]; // Don't hardcode this value here
};

typedef int (*file_open_func)(struct file *file_ptr, int flags);
typedef int (*file_close_func)(struct file *file_ptr);
typedef ssize_t (*file_read_func)(struct file *file_ptr, char *buf, size_t count, unsigned long pos);
typedef ssize_t (*file_write_func)(struct file *file_ptr, const char *buf, size_t count, unsigned long pos);
typedef int (*readdirent_type_t)(struct file *file_ptr, struct dirent *dirp, unsigned int count);

struct file_ops {
    file_open_func open;
    file_close_func close;
    file_read_func read;
    file_write_func write;
    readdirent_type_t readdirent;
};

struct open_file {
    struct file *file;
    unsigned long pos;
};

struct file *file_file_new(void);
void file_file_free(struct file *ptr);
void file_file_ref(struct file *ptr);
void file_file_unref(struct file *ptr);

void file_init_plain(struct file *file_ptr, struct mountpoint *mp, ino_t inode);
void file_init_special(struct file *file_ptr, int major, int minor, struct file *backing);
void file_init_directory(struct file *file_ptr, struct mountpoint *mp, ino_t inode);

struct open_file *file_open_file_new(void);
struct open_file *file_open_file_clone(struct open_file *src);
void file_open_file_free(struct open_file *ptr);

struct file *file_open(const char *pathname, int flags);
int file_close(struct file *file_ptr);
ssize_t file_read(struct file *file_ptr, char *buf, size_t count, unsigned long pos);
ssize_t file_write(struct file *file_ptr, const char *buf, size_t count, unsigned long pos);
int file_readdirent(struct file *file_ptr, struct dirent *dirp, unsigned int count);

int sys_open(USER_PTR(char) pathname, int flags);
int sys_close(int fd);
ssize_t sys_read(int fd, USER_PTR(char) buf, size_t count);
ssize_t sys_write(int fd, USER_PTR(char) buf, size_t count);
int sys_readdirent(unsigned int fd, USER_PTR(struct dirent) dirp, unsigned int count);

int sys_chdir(USER_PTR(const char) path);
int sys_fchdir(int fildes);

#endif
