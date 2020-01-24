#include "syscall.h"
#include <stdint.h>
#include <sys/types.h>

void shell(void);

int stdin_fd;
int stdout_fd;
int stderr_fd;

void init(void) {
    stdin_fd = open("Z:asci0", 0);
    stdout_fd = open("Z:asci0", 0);
    stderr_fd = open("Z:asci0", 0);

    pid_t pid = fork();
    if(pid == 0) {
        shell();
        // TODO: exit instead of looping
        while (1)
            ;
    }

    while (1) {
        int status;
        wait(&status);
    }
}

void shell(void) {
    write(stdout_fd, "In shell\n", 9);

    while (1) {
        char tmp;
        if (read(stdin_fd, &tmp, 1) > 0) {
            write(stdout_fd, &tmp, 1);
        }
    }
}
