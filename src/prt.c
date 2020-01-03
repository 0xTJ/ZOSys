#include "prt.h"

#pragma portmode z180

int prt_start_0(uint16_t count, bool interrupt) {
    TMDR0L = count & 0xFF;
    TMDR0H = (count >> 8) & 0xFF;
    RLDR0L = (count - 1) & 0xFF;
    RLDR0H = ((count - 1) >> 8) & 0xFF;
    TCR |= (interrupt ? __IO_TCR_TIE0 : 0) | __IO_TCR_TDE0;
}

int prt_start_1(uint16_t count, bool interrupt, enum prt_output_control toc) {
    TMDR1L = count & 0xFF;
    TMDR1H = (count >> 8) & 0xFF;
    RLDR1L = (count - 1) & 0xFF;
    RLDR1H = ((count - 1) >> 8) & 0xFF;
    TCR |= (interrupt ? __IO_TCR_TIE1 : 0) | toc | __IO_TCR_TDE1;
}

int prt_stop_0(void) {
    TCR = TCR & ~__IO_TCR_TDE0;
}

int prt_stop_1(void) {
    TCR = TCR & ~__IO_TCR_TDE1;
}
