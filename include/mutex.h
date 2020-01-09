#ifndef INCLUDE_MUTEX_H
#define INCLUDE_MUTEX_H

#include "process.h"
#include <adt.h>
#include <stdbool.h>

typedef struct {
    p_list_t blocked_list;
    bool locked;
} mutex_t;

void mutex_init(mutex_t *mtx);
void mutex_lock(mutex_t *mtx);
bool mutex_trylock(mutex_t *mtx);
void mutex_unlock(mutex_t *mtx);

#endif
