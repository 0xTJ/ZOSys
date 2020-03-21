#ifndef __STDARG_H__
#define __STDARG_H__

#ifndef _VA_LIST_DEFINED
#define _VA_LIST_DEFINED
typedef void * va_list;
#endif

#define va_start(ap, argN) do { ap = ((void *) &argN) + sizeof(argN) } while (0)
#define va_copy(dest, src) do { dest = src } while (0)
#define va_arg(ap, type) (ap += sizeof(type), *(type *) (ap - sizeof(type)))
#define va_end(ap) do {} while (0)

#endif
