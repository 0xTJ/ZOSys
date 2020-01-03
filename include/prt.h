#ifndef INCLUDE_PRT_H
#define INCLUDE_PRT_H

#include <cpu.h>
#include <stdbool.h>
#include <stdint.h>

enum prt_output_control {
    PRT_TOC_INHIBITED = 0,
    PRT_TOC_TOGGLED = __IO_TCR_TOC0,
    PRT_TOC_ZERO = __IO_TCR_TOC1,
    PRT_TOC_ONE = __IO_TCR_TOC1 | __IO_TCR_TOC0
};

int prt_start_0(uint16_t count, bool interrupt);
int prt_start_1(uint16_t count, bool interrupt, enum prt_output_control toc);
int prt_stop_0(void);
int prt_stop_1(void);

#endif
