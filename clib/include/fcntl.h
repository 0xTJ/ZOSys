#ifndef __FCNTL_H__
#define __FCNTL_H__

#define O_EXEC 0x04
#define O_RDONLY 0x01
#define O_RDWR 0x03
#define O_SEARCH 0x04
#define O_WRONLY 0x02

#ifndef _MODE_T_DEFINED
#define _MODE_T_DEFINED
typedef unsigned int mode_t;
#endif

#ifndef _OFF_T_DEFINED
#define _OFF_T_DEFINED
typedef signed long off_t;
#endif

#ifndef _PID_T_DEFINED
#define _PID_T_DEFINED
typedef signed int pid_t;
#endif

int creat(const char *pathname, mode_t mode);
int fcntl(int fd, int cmd, ... /* arg */ );
int open(const char *pathname, int flags, ... /* mode_t mode */ );
int openat(int dirfd, const char *pathname, int flags, ... /* mode_t mode */ );

#endif
