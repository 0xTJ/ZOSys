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
    }

    out("\nforking\n");
    pid_t pid = fork();
    if(pid == 0) {
        out("forked\n");
        // if (execve(argv[0], argv, envp) < 0) {
        //     err("Failed execve\n");
        // }
        return -1;
    }

    out("parent\n");

    return 0;
}

int main(int argc, char **argv, char **envp) {
    while (1) {
        const char prompt[] = "> ";
        write(STDOUT_FILENO, prompt, strlen(prompt));

        char buf[80];
        get_command_line(buf, sizeof(buf));
        out(buf);
        out("doing call_func");
        if (call_func(buf, envp) == -1) {
            out("call_func returned -1\n");
            _exit(1);
        }

        int status = 0;
        out("Waiting\n");
        wait(&status);
        out("Waited: ");
        itoa(status, buf, 10);
        out(buf);
        out("\n");
    }
}
