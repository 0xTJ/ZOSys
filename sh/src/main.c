#include "syscall.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>

void shell(void);

int stdin_fd;
int stdout_fd;
int stderr_fd;

int main(int argc, char **argv, char **envp) {
    const char prompt[] = "> ";
    write(stdout_fd, prompt, strlen(prompt));

    while (1) {
        char tmp;
        if (read(stdin_fd, &tmp, 1) > 0) {
            if (tmp == 'x') {
                const char *exit_message = "\nExiting\n";
                write(stdout_fd, exit_message, strlen(exit_message));
                return 0;
            }
            write(stdout_fd, &tmp, 1);
        }
    }
}
