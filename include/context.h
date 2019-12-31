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

extern volatile uint16_t context_temp_sp;
extern volatile uint8_t context_temp_bbr;

extern void context_init(void (*pc)());

#endif
