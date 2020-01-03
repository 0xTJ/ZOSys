#ifndef INCLUDE_MUTEX_H
#define INCLUDE_MUTEX_H

typedef unsigned char mutex_t;
#define MUTEX_INIT ((mutex_t) 0)

void mutex_lock(mutex_t *mtx) __z88dk_fastcall;
void mutex_unlock(mutex_t *mtx) __z88dk_fastcall;

#endif
