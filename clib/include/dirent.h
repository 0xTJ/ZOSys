#ifndef INCLUDE_DIRENT_H
#define INCLUDE_DIRENT_H

// typedef __ DIR;

#ifndef _INO_T_DEFINED
#define _INO_T_DEFINED
typedef unsigned long ino_t;
#endif

struct dirent {
    ino_t d_ino;
    char d_name[256];
};

// TODO: Complete this file

int readdirent(int fd, struct dirent *dirp, unsigned int count);

#endif
