#include "dma.h"
#include <cpu.h>
#include <z88dk.h>

#pragma portmode z180

mutex_t dma_0_mtx;

void dma_0_init(void) {
    mutex_init(&dma_0_mtx);
}

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

int dma_memcpy(uint32_t dest, uint32_t src, uint16_t num) {
    mutex_lock(&dma_0_mtx);
    dma_0_addr(src, dest, num);
    dma_0_mode(MEMORY_INC, MEMORY_INC, true);
    dma_0_enable();
    // Burst mode, so no need to check completion
    mutex_unlock(&dma_0_mtx);
    return 0;
}

void int_dma0(void) {
    // TODO: Implement this
}

void int_dma1(void) {
    // TODO: Implement this
}
