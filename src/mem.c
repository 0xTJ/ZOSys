#include "mem.h"
#include "dma.h"
#include <cpu.h>
#include <intrinsic.h>
#include <string.h>
 
#pragma portmode z180

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

USER_PTR(void) mem_memcpy_user_from_kconst(USER_PTR(void) dest, const void *src, size_t count) {
    dma_memcpy(pa_from_pfn(CBR) + (uintptr_t) dest, 0x00000U + (uintptr_t) src, count);
    return dest;
}

USER_PTR(void) mem_memcpy_user_from_kdata(USER_PTR(void) dest, const void *src, size_t count) {
    dma_memcpy(pa_from_pfn(CBR) + (uintptr_t) dest, pa_from_pfn(BBR) + (uintptr_t) src, count);
    return dest;
}

USER_PTR(void) mem_memcpy_user_from_kstack(USER_PTR(void) dest, const void *src, size_t count) {
    dma_memcpy(pa_from_pfn(CBR) + (uintptr_t) dest, pa_from_pfn(CBR) + (uintptr_t) src, count);
    return dest;
}

USER_PTR(void) mem_memcpy_user_from_kernel(USER_PTR(void) dest, const void *src, size_t count) {
    if ((uintptr_t) src >= 0xF000) {
        return mem_memcpy_user_from_kstack(dest, src, count);
    } else if ((uintptr_t) src >= 0x1000) {
        return mem_memcpy_user_from_kdata(dest, src, count);
    } else {
        return mem_memcpy_user_from_kconst(dest, src, count);
    }
}

void *mem_memcpy_kdata_from_user(void *dest, USER_PTR(const void) src, size_t count) {
    dma_memcpy(pa_from_pfn(BBR) + (uintptr_t) dest, pa_from_pfn(CBR) + (uintptr_t) src, count);
    return dest;
}

void *mem_memcpy_kstack_from_user(void *dest, USER_PTR(const void) src, size_t count) {
    dma_memcpy(pa_from_pfn(CBR) + (uintptr_t) dest, pa_from_pfn(CBR) + (uintptr_t) src, count);
    return dest;
}

void *mem_memcpy_kernel_from_user(void *dest, USER_PTR(const void) src, size_t count) {
    if ((uintptr_t) dest >= 0xF000) {
        return mem_memcpy_kstack_from_user(dest, src, count);
    } else if ((uintptr_t) dest >= 0x1000) {
        return mem_memcpy_kdata_from_user(dest, src, count);
    } else {
        return dest;
    }
}

USER_PTR(void) mem_memcpy_user_from_user(USER_PTR(void) dest, USER_PTR(const void) src, size_t count) {
    dma_memcpy(pa_from_pfn(CBR) + (uintptr_t) dest, pa_from_pfn(CBR) + (uintptr_t) src, count);
    return dest;
}

void *mem_get_user_buffer(void) {
    return user_buffer;
}

void *mem_copy_to_user_buffer(USER_PTR(void) user_ptr, size_t count) {
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

void *mem_copy_from_user_buffer(USER_PTR(void) user_ptr, size_t count) {
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

int mem_user_strlen(USER_PTR(const char) user_ptr) {
    if (!mem_user_valid_ptr(user_ptr)) {
        return -1;
    }

    mem_copy_to_user_buffer(user_ptr, MEM_USER_BUFFER_SIZE);

    size_t len = strnlen(user_buffer, MEM_USER_BUFFER_SIZE);

    if (len == MEM_USER_BUFFER_SIZE) {
        return -1;
    } else {
        return len;
    }
}

int mem_user_vector_len(USER_PTR(USER_PTR(void)) user_vector) {
    // Chack that front of list is in valid space
    if (!mem_user_valid_ptr(user_vector)) {
        return -1;
    }

    USER_PTR(void) *copied_vector = mem_copy_to_user_buffer(user_vector, MEM_USER_BUFFER_SIZE);

    unsigned int null_index = 0;
    bool found_null = false;
    const unsigned int max_pointers = MEM_USER_BUFFER_SIZE / sizeof(char *);
    while (null_index < max_pointers) {
        // Chack that back of this pointer is in valid space
        if (!mem_user_valid_ptr((uintptr_t) user_vector + (null_index + 1) * sizeof(USER_PTR(void)) - 1)) {
            return -1;
        }
        if (!copied_vector[null_index]) {
            found_null = true;
            break;
        }
        null_index += 1;
    }

    if (!found_null) {
        return -1;
    } else {
        return null_index;
    }
}

bool mem_user_valid_ptr(USER_PTR(void) ptr) {
    return (ptr >= 0x1000) && (ptr < 0xF000);
}
