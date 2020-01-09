#ifndef INCLUDE_spinlock_H
#define INCLUDE_spinlock_H

#include "process.h"
#include <adt.h>
#include <stdbool.h>

typedef struct {
    unsigned char locked;
} spinlock_t;

void spinlock_init(spinlock_t *spnlk);
void spinlock_lock(spinlock_t *spnlk) __z88dk_fastcall;
bool spinlock_trylock(spinlock_t *spnlk) __z88dk_fastcall;
void spinlock_unlock(spinlock_t *spnlk) __z88dk_fastcall;

#endif
