#ifndef INCLUDE_SEMAPHORE_H
#define INCLUDE_SEMAPHORE_H

#include "process.h"
#include <adt.h>
#include <stdbool.h>

typedef struct {
    p_list_t blocked_list;
    signed int count;
} semaphore_t;

void semaphore_init(semaphore_t *smphr);
void semaphore_lock(semaphore_t *smphr);
bool semaphore_trylock(semaphore_t *smphr);
void semaphore_unlock(semaphore_t *smphr);

#endif
