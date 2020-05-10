#include "prt.h"
#include <cpu.h>
#include <intrinsic.h>
#include <stddef.h>
#include <stdint.h>

#pragma portmode z180

void (*interrupt_routine_0)(void) = NULL;
void (*interrupt_routine_1)(void) = NULL;

int prt_start_0(uint16_t count, bool interrupt) {
    TCR = TCR & ~__IO_TCR_TDE0;
    TMDR0L = count & 0xFF;
    TMDR0H = (count >> 8) & 0xFF;
    RLDR0L = (count - 1) & 0xFF;
    RLDR0H = ((count - 1) >> 8) & 0xFF;
    TCR |= (interrupt ? __IO_TCR_TIE0 : 0) | __IO_TCR_TDE0;
    return 0;
}

int prt_start_1(uint16_t count, bool interrupt, enum prt_output_control toc) {
    TCR = TCR & ~__IO_TCR_TDE1;
    TMDR1L = count & 0xFF;
    TMDR1H = (count >> 8) & 0xFF;
    RLDR1L = (count - 1) & 0xFF;
    RLDR1H = ((count - 1) >> 8) & 0xFF;
    TCR |= (interrupt ? __IO_TCR_TIE1 : 0) | toc | __IO_TCR_TDE1;
    return 0;
}

int prt_stop_0(void) {
    TCR = TCR & ~__IO_TCR_TDE0;
    return 0;
}

int prt_stop_1(void) {
    TCR = TCR & ~__IO_TCR_TDE1;
    return 0;
}

void prt_interrupt_routine_0(void (*interrupt_routine)(void)) __critical {
    interrupt_routine_0 = interrupt_routine;
}

void prt_interrupt_routine_1(void (*interrupt_routine)(void)) __critical {
    interrupt_routine_1 = interrupt_routine;
}

void int_prt0(void) {
    (void) TCR;
    (void) TMDR0L;

    if (interrupt_routine_0)
        interrupt_routine_0();
}

void int_prt1(void) {
    (void) TCR;
    (void) TMDR1L;

    if (interrupt_routine_1)
        interrupt_routine_1();
}
