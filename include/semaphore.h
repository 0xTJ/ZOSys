#ifndef INCLUDE_SEMAPHORE_H
#define INCLUDE_SEMAPHORE_H

#include "process.h"
#include <adt.h>
#include <stdbool.h>

typedef struct {
    p_list_t blocked_list;
    signed int count;
} semaphore_t;

void semaphore_init(semaphore_t *smphr, signed int initial_value);
void semaphore_wait(semaphore_t *smphr);
bool semaphore_trywait(semaphore_t *smphr);
void semaphore_signal(semaphore_t *smphr);

#endif
