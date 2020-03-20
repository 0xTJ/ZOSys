#ifndef __STDIO_H__
#define __STDIO_H__

typedef struct {
    int fd;
} FILE;

typedef unsigned long fpos_t;

#ifndef _OFF_T_DEFINED
#define _OFF_T_DEFINED
typedef signed long off_t;
#endif

#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
typedef unsigned int size_t;
#endif

#ifndef _SSIZE_T_DEFINED
#define _SSIZE_T_DEFINED
typedef signed int ssize_t;
#endif

#endif
