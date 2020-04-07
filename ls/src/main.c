#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv, char **envp) {
    const char *path = argv[1];

    int dir_fd = open(path, O_RDONLY);
    if (dir_fd < 0) {
        return -1;
    }

    struct dirent dent;
    for (unsigned int i = 0; i < (unsigned int) -1; ++i) {
        int status = readdirent(dir_fd, &dent, i);
        if (status == -1) {
            return -1;
        }
        write(STDERR_FILENO, dent.d_name, strlen(dent.d_name));
        write(STDOUT_FILENO, "\n", 1);
        if (status == 0) {
            break;
        }
    }

    return 0;
}
