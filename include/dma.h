#ifndef INCLUDE_DMA_H
#define INCLUDE_DMA_H

#include "mutex.h"
#include <stdbool.h>
#include <stdint.h>

enum dma_0_mode {
    MEMORY_INC = 0,
    MEMORY_DEC = 1,
    MEMORY_FIX = 2,
    IO_FIX = 3,
};

extern mutex_t dma_0_mtx;

void dma_0_init(void);
int dma_0_addr(unsigned long src_addr, unsigned long dest_addr, unsigned int byte_count);
int dma_0_mode(enum dma_0_mode src_mode, enum dma_0_mode dest_mode, bool burst_mode);
int dma_0_enable(void);
int dma_memcpy(uint32_t dest, uint32_t src, uint16_t num);

#endif
