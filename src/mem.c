#include "mem.h"
#include "dma.h"
#include <cpu.h>
#include <intrinsic.h>

#define PAGE_COUNT 256U
#define PAGES_PER_BLOCK 16U
#define PAGE_BLOCK_COUNT (PAGE_COUNT / PAGES_PER_BLOCK)

// Size here must match size in syscall.c
extern char user_buffer[MEM_USER_BUFFER_SIZE];

uint16_t page_usage_map[PAGE_BLOCK_COUNT] = {
        0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    };

int mem_alloc_page(void) __critical {
    // First try to grab from partially used page block
    for (unsigned char page_block = 0; page_block < PAGE_BLOCK_COUNT; ++page_block) {
        if (page_usage_map[page_block] != 0xFFFF && page_usage_map[page_block] != 0x0000) {
            // Partially used page block, use it
            for (unsigned char page_in_block = 0; page_in_block < PAGES_PER_BLOCK; ++page_in_block) {
                if (!(page_usage_map[page_block] & (1U << page_in_block))) {
                    page_usage_map[page_block] |= 1U << page_in_block;
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
            return page_block * PAGE_COUNT + page_in_block;
        }
    }

    return -1;
}

int mem_alloc_page_block(void) __critical {
    for (unsigned char page_block = 0; page_block < PAGE_BLOCK_COUNT; ++page_block) {
        if (page_usage_map[page_block] == 0x0000) {
            // Empty page block, use it
            page_usage_map[page_block] = 0xFFFF;
            unsigned int page = page_block * PAGES_PER_BLOCK;
            return page;
        }
    }

    return -1;
}

int mem_alloc_page_block_specific(unsigned int page) __critical {
    if (page % PAGES_PER_BLOCK == 0) {
        unsigned int page_block = page / PAGES_PER_BLOCK;
        if (page_block < PAGE_BLOCK_COUNT) {
            if (page_usage_map[page_block] == 0x0000) {
                // Empty page block, use it
                page_usage_map[page_block] = 0xFFFF;
                return page;
            }
        }
    }

    return -1;
}

void mem_free_page(unsigned char page) __critical {
    unsigned char page_block = page / PAGES_PER_BLOCK;
    unsigned char page_in_block = page % PAGES_PER_BLOCK;
    page_usage_map[page_block] &= ~(1U << page_in_block);
}

void mem_free_page_block(unsigned char page) __critical {
    unsigned char page_block = page / PAGES_PER_BLOCK;
    page_usage_map[page_block] = 0x0000;
}

char *mem_get_user_buffer(void) {
    return user_buffer;
}

char *mem_copy_to_user_buffer(uintptr_t user_ptr, size_t count) {
    if (count > sizeof(user_buffer)) {
        // panic();
        // TODO: Add panic() and use instead of the following
        count = sizeof(user_buffer);
    }

    uint32_t user_addr_base = pa_from_pfn(CBR);
    uint32_t user_buffer_addr = user_addr_base + (uintptr_t) user_buffer;
    uint32_t source_addr = user_addr_base + user_ptr;

    dma_memcpy(user_buffer_addr, source_addr, count);

    return user_buffer;
}

char *mem_copy_from_user_buffer(uintptr_t user_ptr, size_t count) {
    if (count > sizeof(user_buffer)) {
        // panic();
        // TODO: Add panic() and use instead of the following
        count = sizeof(user_buffer);
    }

    uint32_t user_addr_base = pa_from_pfn(CBR);
    uint32_t user_buffer_addr = user_addr_base + (uintptr_t) user_buffer;
    uint32_t dest_addr = user_addr_base + user_ptr;

    dma_memcpy(dest_addr, user_buffer_addr, count);

    return user_buffer;
}
