#include "syscall.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
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
        int wstatus;
        pid_t wpid = wait(&wstatus);
        if (wpid == -1) {
            const char *done_message = "\nAll non-init processes have exited\n";
            write(stdout_fd, done_message, strlen(done_message));
            _exit(0);
        }
    }
}

void shell(void) {
    const char prompt[] = "> ";
    write(stdout_fd, prompt, strlen(prompt));

    while (1) {
        char tmp;
        if (read(stdin_fd, &tmp, 1) > 0) {
            if (tmp == 'x')
                _exit(0);
            write(stdout_fd, &tmp, 1);
        }
    }
}
