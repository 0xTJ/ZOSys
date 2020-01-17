#include "io_system.h"
// #include <arch/scz180.h>

mutex_t io_system_mtx;

void io_system_init(void) {
    // io_system = IO_SYSTEM_PASSIVE;
    mutex_init(&io_system_mtx);
}
