#include "syscall.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>

void shell(void);

int stdin_fd;
int stdout_fd;
int stderr_fd;

void main(int argc, char **argv, char **envp) {
    stdin_fd = open("Z:asci0", 0);
    stdout_fd = open("Z:asci0", 0);
    stderr_fd = open("Z:asci0", 0);

    const char *start_message = "Init has started\n";
    write(stdout_fd, start_message, strlen(start_message));

    write(stdout_fd, "argv:\n", 6);
    for (int i = 0; i < argc; ++i) {
        write(stdout_fd, "    ", 4);
        write(stdout_fd, argv[i], strlen(argv[i]));
        write(stdout_fd, "\n", 1);
    }

    write(stdout_fd, "envp:\n", 6);
    for (int i = 0;; ++i) {
        if (!envp[i]) {
            break;
        }
        write(stdout_fd, "    ", 4);
        write(stdout_fd, envp[i], strlen(envp[i]));
        write(stdout_fd, "\n", 1);
    }

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
            const char *done_message = "All non-init processes have exited\n";
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
            if (tmp == 'x') {
                const char *exit_message = "\nExiting\n";
                write(stdout_fd, exit_message, strlen(exit_message));
                _exit(0);
            }
            write(stdout_fd, &tmp, 1);
        }
    }
}
