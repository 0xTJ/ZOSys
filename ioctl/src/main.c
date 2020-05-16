#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <unistd.h>

void out(const char *s) {
    write(STDOUT_FILENO, s, strlen(s));
}

void err(const char *s) {
    write(STDERR_FILENO, s, strlen(s));
}

int main(int argc, char **argv) {
    if (argc < 3) {
        err("Usage:\n");
        err(argv[0]);
        err(" <device> <request> [<arg>]\n");
        return 1;
    }

    int fd = open(argv[1], O_RDWR);
    if (fd < 0) {
        err("Failed to open \"");
        err(fd);
        err("\"\n");
        return 1;
    }

    int request = atoi(argv[2]);

    uintptr_t arg = 0;
    if (argc > 3) {
        arg = atoi(argv[3]);
    }

    ioctl(fd, request, arg);

    return 0;
}
