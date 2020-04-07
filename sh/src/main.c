#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv, char **envp) {
    const char prompt[] = "> ";
    write(STDOUT_FILENO, prompt, strlen(prompt));

    pid_t pid = fork();
    if(pid == 0) {
        write(STDOUT_FILENO, "ls\n", 3);
        char *ls_argv[] = { "ls", "Z:", NULL };
        char *ls_envp[] = { "PATH=\"Y:\"", NULL };
        if (execve("Y:ls", ls_argv, ls_envp) < 0) {
            const char *tmp = "Failed execve\n";
            write(STDOUT_FILENO, tmp, strlen(tmp));
        }
    }

    while (1) {
        char tmp;
        if (read(STDIN_FILENO, &tmp, 1) > 0) {
            if (tmp == 'x') {
                const char *exit_message = "\nExiting\n";
                write(STDOUT_FILENO, exit_message, strlen(exit_message));
                _exit(0);
            }
            write(STDOUT_FILENO, &tmp, 1);
        }
    }
}
