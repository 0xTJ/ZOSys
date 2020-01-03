#ifndef INCLUDE_DMA_H
#define INCLUDE_DMA_H

#include <stdbool.h>

enum dma_0_mode {
    MEMORY_INC = 0,
    MEMORY_DEC = 1,
    MEMORY_FIX = 2,
    IO_FIX = 3,
};

int dma_addr_0(unsigned long src_addr, unsigned long dest_addr, unsigned int byte_count);
int dma_mode_0(enum dma_0_mode src_mode, enum dma_0_mode dest_mode, bool burst_mode);
int dma_enable_0(void);

#endif
