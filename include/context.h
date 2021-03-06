#ifndef INCLUDE_CONTEXT_H
#define INCLUDE_CONTEXT_H

#include <stdint.h>

struct context_stack {
    uint16_t iy;
    uint16_t hl_;
    uint16_t de_;
    uint16_t bc_;
    uint16_t hl;
    uint16_t de;
    uint16_t bc;
    uint16_t af_;
    uint16_t af;
    uint16_t ix;
    uint16_t pc;
};

extern void context_init(void (*pc)());
// The following functions can not be called from C, they will break the stack

#endif
