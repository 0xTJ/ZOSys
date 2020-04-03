#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/wait.h>

int main(int argc, char **argv, char **envp) {
    const char prompt[] = "> ";
    write(STDOUT_FILENO, prompt, strlen(prompt));

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
