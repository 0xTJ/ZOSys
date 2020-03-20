#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

void shell(void);

int stdin_fd;
int stdout_fd;
int stderr_fd;

int main(int argc, char **argv, char **envp) {
    stdin_fd = open("Z:asci0", O_RDONLY);
    stdout_fd = open("Z:asci0", O_WRONLY);
    stderr_fd = open("Z:asci0", O_WRONLY);

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
        if (execve("Y:sh", argv, envp) < 0) {
            const char *tmp = "Failed execve\n";
            write(stdout_fd, tmp, strlen(tmp));
        }
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
            return 0;
        }
    }
}
