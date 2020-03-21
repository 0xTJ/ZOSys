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

#ifndef _VA_LIST_DEFINED
#define _VA_LIST_DEFINED
typedef void * va_list;
#endif

#define _IOFBF 0x01
#define _IOLBF 0x02
#define _IOLBF 0x03

#define SEEK_CUR 0x01
#define SEEK_END 0x02
#define SEEK_SET 0x03

#define FILENAME_MAX 32
#define FOPEN_MAX 8

#define EOF ((int) -1)

#ifndef NULL
#define NULL ((void *) 0)
#endif

extern FILE __stderr;
extern FILE __stdin;
extern FILE __stdout;

#define stderr (&__stderr)
#define stdin (&__stdin)
#define stdout (&__stdout)

#endif
