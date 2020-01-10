#include "mem.h"
#include <cpu.h>
#include <intrinsic.h>

#define PAGE_COUNT 256
#define PAGES_PER_BLOCK 16
#define PAGE_BLOCK_COUNT (PAGE_COUNT / PAGES_PER_BLOCK)

uint16_t page_usage_map[PAGE_BLOCK_COUNT] = {
        0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
        0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    };

int mem_alloc_page(void) {
    uint8_t int_state = cpu_get_int_state();
    intrinsic_di();

    // First try to grab from partially used page block
    for (unsigned char page_block = 0; page_block < PAGE_BLOCK_COUNT; ++page_block) {
        if (page_usage_map[page_block] != 0xFFFF && page_usage_map[page_block] != 0x0000) {
            // Partially used page block, use it
            for (unsigned char page_in_block = 0; page_in_block < PAGES_PER_BLOCK; ++page_in_block) {
                if (!(page_usage_map[page_block] & (1U << page_in_block))) {
                    page_usage_map[page_block] |= 1U << page_in_block;
                    cpu_set_int_state(int_state);
                    return page_block * PAGE_COUNT + page_in_block;
                }
            }
        }
    }

    // Try to grab from an empty page block
    for (unsigned char page_block = 0; page_block < PAGE_BLOCK_COUNT; ++page_block) {
        if (page_usage_map[page_block] == 0x0000) {
            // Empty page block, use it
            unsigned char page_in_block = 0;
            page_usage_map[page_block] |= 1U << page_in_block;
            cpu_set_int_state(int_state);
            return page_block * PAGE_COUNT + page_in_block;
        }
    }

    cpu_set_int_state(int_state);
    return -1;
}

int mem_alloc_page_block(void) {
    uint8_t int_state = cpu_get_int_state();
    intrinsic_di();

    for (unsigned char page_block = 0; page_block < PAGE_BLOCK_COUNT; ++page_block) {
        if (page_usage_map[page_block] == 0x0000) {
            // Empty page block, use it
            page_usage_map[page_block] = 0xFFFF;
            cpu_set_int_state(int_state);
            return page_block * PAGES_PER_BLOCK;
        }
    }

    cpu_set_int_state(int_state);
    return -1;
}
