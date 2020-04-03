#ifndef __UNISTD_H__
#define __UNISTD_H__

#ifndef NULL
#define NULL ((void*) (0))
#endif

#ifndef SEEK_SET
#define SEEK_SET 0
#endif

#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif

#ifndef SEEK_END
#define SEEK_END 2
#endif

#define STDERR_FILENO 2
#define STDIN_FILENO 0
#define STDOUT_FILENO 1

#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
typedef unsigned int size_t;
#endif

#ifndef _SSIZE_T_DEFINED
#define _SSIZE_T_DEFINED
typedef signed int ssize_t;
#endif

#ifndef _UID_T_DEFINED
#define _UID_T_DEFINED
typedef signed int uid_t;
#endif

#ifndef _GID_T_DEFINED
#define _GID_T_DEFINED
typedef signed int gid_t;
#endif

#ifndef _OFF_T_DEFINED
#define _OFF_T_DEFINED
typedef signed long off_t;
#endif

#ifndef _PID_T_DEFINED
#define _PID_T_DEFINED
typedef signed int pid_t;
#endif

#ifndef _INTPTR_T_DEFINED
#define _INTPTR_T_DEFINED
typedef signed int intptr_t;
#endif

int chdir(const char *path);
int close(int fd);
int dup(int oldfd);
int dup2(int oldfd, int newfd);
int execve(const char *pathname, char *const argv[], char *const envp[]);
void _exit(int status);
int fchdir(int fildes);
pid_t fork(void);
off_t lseek(int fd, void *buf, size_t count);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);

#endif
