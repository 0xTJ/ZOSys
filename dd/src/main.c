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

int main(int argc, char **argv, char **envp) {
    if (argc < 1) {
        return 1;
    }

    const char *if_str = NULL;
    const char *of_str = NULL;

    for (unsigned int i = 1; argv[i]; ++i) {
        if (strncmp("if=", argv[i], 3) == 0) {
            if_str = argv[i] + 3;
        } else if (strncmp("of=", argv[i], 3) == 0) {
            of_str = argv[i] + 3;
        } else {
            err("Unknown argument: \"");
            err(argv[i]);
            err("\"\n");
        }
    }

    int if_fd;
    if (if_str) {
        if_fd = open(if_str, O_RDONLY);
    } else {
        if_fd = STDIN_FILENO;
    }

    int of_fd;
    if (of_str) {
        of_fd = open(of_str, O_WRONLY);
    } else {
        of_fd = STDOUT_FILENO;
    }

    if (if_fd < 0) {
        err("Failed to open \"");
        err(if_str);
        err("\" for reading\n");
        return 1;
    }
    if (of_fd < 0) {
        err("Failed to open \"");
        err(of_str);
        err("\" for writing\n");
        return 1;
    }

    while (1) {
        char tmp[256];
        ssize_t read_result = read(if_fd, tmp, sizeof(tmp));
        if (read_result < 0) {
            break;
        }
        ssize_t write_result = write(of_fd, tmp, read_result);
        if (write_result < read_result) {
            break;
        }
        // TODO: Improve this
    }

    return 0;
}
