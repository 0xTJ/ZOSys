#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

void out(const char *s) {
    write(STDOUT_FILENO, s, strlen(s));
}

void err(const char *s) {
    write(STDERR_FILENO, s, strlen(s));
}

ssize_t get_command_line(char *buf, size_t buflen) {
    size_t index = 0;

    while (1) {
        char read_char;
        if (read(STDIN_FILENO, &read_char, 1) < 1) {
            continue;
        }

        write(STDOUT_FILENO, &read_char, 1); // TODO: Check for error

        switch (read_char) {
        case '\n':
        case '\r':
            return index;
        case '\b':
            if (index > 0) {
                index -= 1;
            }
            break;
        default:
            buf[index++] = read_char;
        }
        if (index >= buflen) {
            index = buflen - 1;
        }
        buf[index] = '\0';
    }
}

int call_func(char *command_line, char **envp) {

    out("command_line:\n");
    out(command_line);

    char *argv[8] = {0};
    char *ptr_to_use = command_line;
    for (unsigned i = 0;; ++i) {
        char *tok = strtok(ptr_to_use, " \t\r\n");
        ptr_to_use = NULL;
        argv[i] = tok;
        if (!tok) {
            break;
        }
    }

    if (strcmp(argv[0], "exit") == 0) {
        int status = 0;
        if (argv[1] != NULL) {
            status = atoi(argv[1]);
        }
        _exit(status);
    } else if (strcmp(argv[0], "cd") == 0) {
        if (argv[1]) {
            chdir(argv[1]);
        } else {
            err("cd requires an argument\n");
        }
        return 0;
    }

    out("\nforking\n");
    pid_t pid = fork();
    if(pid == 0) {
        out("forked\n");
        if (execve(argv[0], argv, envp) < 0) {
            err("Failed execve\n");
            _exit(1);
        }
        return -1;
    } else if (pid < 0) {
        out("Failed to fork\n");
    } else {
        out("parent\n");
        int child_status = 0;
        out("Waiting\n");
        wait(&child_status);
        out("Waited: ");
        char buf[16];
        itoa(child_status, buf, 10);
        out(buf);
        out("\n");
    }

    return 0;
}

int main(int argc, char **argv, char **envp) {
    while (1) {
        const char prompt[] = "> ";
        write(STDOUT_FILENO, prompt, strlen(prompt));

        char buf[80];
        get_command_line(buf, sizeof(buf));
        out(buf);
        out("doing call_func\n");
        call_func(buf, envp);
    }
}
