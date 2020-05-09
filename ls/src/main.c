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
    // const char *path = argv[1];
    // if (!path) {
    //     path = "";
    // }

    // int dir_fd = open(path, O_RDONLY);
    // if (dir_fd < 0) {
    //     err("Failed to open path\n");
    //     return -1;
    // }

    // struct dirent dent;
    // for (unsigned int i = 0; i < (unsigned int) -1; ++i) {
    //     int status = readdirent(dir_fd, &dent, i);
    //     if (status == -1) {
    //         return -1;
    //     }
    //     out(dent.d_name);
    //     out("\n");
    //     if (status == 0) {
    //         break;
    //     }
    // }

    return 0;
}
