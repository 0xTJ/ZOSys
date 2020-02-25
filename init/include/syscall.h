#ifndef INCLUDE_SYSCALL_H
#define INCLUDE_SYSCALL_H

#include <stdint.h>
#include <sys/types.h>


int fork(void);
pid_t wait(int *wstatus);
pid_t waitpid(pid_t pid, int *wstatus, int options);
void _exit(int status);
int open(const char *pathname, int flags);
int close(int fd);
ssize_t read(int fd, char *buf, size_t count);
ssize_t write(int fd, const char *buf, size_t count);
int execve(const char *pathname, char *const argv[], char *const envp[]);

#endif
