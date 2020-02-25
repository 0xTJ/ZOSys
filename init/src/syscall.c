#include "syscall.h"

pid_t fork(void) __naked {
    __asm__("ld a, 0\nrst 8\nret");
}

pid_t wait(int *wstatus) {
    return waitpid(-1, wstatus, 0);
}

pid_t waitpid(pid_t pid, int *wstatus, int options) __naked {
    __asm__("ld a, 1\nrst 8\nret");
    (void) pid, (void) wstatus, (void) options;
}

void _exit(int status) __naked {
    __asm__("ld a, 2\nrst 8\nret");
    (void) status;
}

int open(const char *pathname, int flags) __naked {
    __asm__("ld a, 3\nrst 8\nret");
    (void) pathname, (void) flags;
}

int close(int fd) __naked {
    __asm__("ld a, 4\nrst 8\nret");
    (void) fd;
}

ssize_t read(int fd, char *buf, size_t count) __naked {
    __asm__("ld a, 5\nrst 8\nret");
    (void) fd, (void) buf, (void) count;
}

ssize_t write(int fd, const char *buf, size_t count) __naked {
    __asm__("ld a, 6\nrst 8\nret");
    (void) fd, (void) buf, (void) count;
}

int execve(const char *pathname, char *const argv[], char *const envp[]) __naked {
    __asm__("ld a, 6\nrst 8\nret");
    (void) pathname, (void) argv, (void) envp;
}

