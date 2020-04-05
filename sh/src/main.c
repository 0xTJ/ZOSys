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

    int sd0_fd = open("Z:sd0", O_RDONLY);

    char buf[512];
    read(sd0_fd, buf, sizeof(buf));
    write(STDOUT_FILENO, "\n", 1);
    for (unsigned i = 0; i < sizeof(buf); ++i) {
        char str_buf[3];
        itoa(buf[i], str_buf, 16);
        if (str_buf[1] == '\0') {
            str_buf[2] = '\0';
            str_buf[1] = str_buf[0];
            str_buf[1] = '0';
        }
        write(STDOUT_FILENO, str_buf, 2);
        if ((i + 1) % 16 != 0) {
            write(STDOUT_FILENO, " ", 1);
        } else {
            write(STDOUT_FILENO, "\n", 1);
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
            write(STDERR_FILENO, &tmp, 1);
        }
    }
}
