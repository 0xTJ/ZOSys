#include <ctype.h>
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
    buf[0] = '\0';

    while (1) {
        char read_char;
        if (read(STDIN_FILENO, &read_char, 1) < 1) {
            continue;
        }
        
        if (!(isprint(read_char) ||
              read_char == '\n' || read_char == '\r' ||
              read_char == '\b' || read_char == '\t')) {
            continue;
        }

        if (read_char != '\b') {
            write(STDOUT_FILENO, &read_char, 1); // TODO: Check for error
        }

        switch (read_char) {
        case '\n':
        case '\r':
            return index;
        case '\b':
            if (index > 0) {
                char backspace_chars[] = { '\b', ' ', '\b' };
                write(STDOUT_FILENO, backspace_chars, sizeof(backspace_chars)); // TODO: Check for error
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

    pid_t pid = fork();
    if(pid == 0) {
        if (execve(argv[0], argv, envp) < 0) {
            err("Failed execve\n");
            _exit(1);
        }
        return -1;
    } else if (pid < 0) {
        out("Failed to fork\n");
    } else {
        int child_status = 0;
        wait(&child_status);
    }

    return 0;
}

int main(int argc, char **argv, char **envp) {
    int pipefd[2];

    if (pipe(pipefd) == -1) {
        err("Failed to open pipe");
    }

    char write_val[] = "tes\n";
    write(pipefd[1], &write_val, 5);
    char read_val[] = "none\n";
    read(pipefd[0], &read_val, 6);

    out(write_val);
    out(read_val);

    dup2(pipefd[1], STDERR_FILENO);

    while (1) {
        const char prompt[] = "> ";
        write(STDOUT_FILENO, prompt, strlen(prompt));

        

        char buf[80];
        get_command_line(buf, sizeof(buf));
        call_func(buf, envp);
    }
}
