#include "dma.h"
#include <cpu.h>
#include <z88dk.h>

#pragma portmode z180

mutex_t dma_0_mtx = MUTEX_INIT;

int dma_0_addr(unsigned long src_addr, unsigned long dest_addr, unsigned int byte_count) {
    SAR0L = (src_addr >> 0) & 0xFF;
    SAR0H = (src_addr >> 8) & 0xFF;
    SAR0B = (src_addr >> 16) & 0x0F;
    DAR0L = (dest_addr >> 0) & 0xFF;
    DAR0H = (dest_addr >> 8) & 0xFF;
    DAR0B = (dest_addr >> 16) & 0x0F;
    BCR0L = (byte_count >> 0) & 0xFF;
    BCR0H = (byte_count >> 8) & 0xFF;

    return 0;
}

int dma_0_mode(enum dma_0_mode src_mode, enum dma_0_mode dest_mode, bool burst_mode) {
    if ((dest_mode == MEMORY_FIX || dest_mode == IO_FIX) && (src_mode == MEMORY_FIX || src_mode == IO_FIX))
        return -1;

    DMODE = dest_mode << 4 | src_mode << 2 | burst_mode << 1;

    return 0;
}

int dma_0_enable(void) {
    DSTAT = (DSTAT & ~__IO_DSTAT_DWE0) | __IO_DSTAT_DE0;

    return 0;
}
