#ifndef INCLUDE_CONTEXT_H
#define INCLUDE_CONTEXT_H

#include <stdint.h>

extern volatile uint16_t context_temp_sp;
extern volatile uint8_t context_temp_bbr;

extern void context_init(void (*pc)());

#endif
