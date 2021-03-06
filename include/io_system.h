#ifndef INCLUDE_IO_SYSTEM_H
#define INCLUDE_IO_SYSTEM_H

#include "mutex.h"

#define IO_SYSTEM_PASSIVE 0xFD

extern mutex_t io_system_mtx;

void io_system_init(void);

#endif
