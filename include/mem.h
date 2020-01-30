#ifndef INCLUDE_MEM_H
#define INCLUDE_MEM_H

#include <stdint.h>
#include <sys/types.h>

#define USER_PTR(type) uintptr_t
#define MEM_USER_BUFFER_SIZE 256

#define PAGE_SHIFT 12

uint8_t pfn_from_pa(uint32_t pa);
#define pfn_from_pa(pa) ((uint8_t) (((pa) & 0xFFFFFF) >> PAGE_SHIFT))
uint32_t pa_from_pfn(uint8_t pfn);
#define pa_from_pfn(pfn) ((uint32_t) (pfn) << PAGE_SHIFT)
uint8_t vpn_from_va(uintptr_t va);
#define vpn_from_va(va) ((uint8_t) ((va) >> PAGE_SHIFT))
uintptr_t va_from_pfn(uint8_t vpn);
#define va_from_pfn(vpn) (((uintptr_t) (vpn) & 0xF) << PAGE_SHIFT)

int mem_alloc_page(void);
int mem_alloc_page_block(void);
int mem_alloc_page_block_specific(unsigned int page);
void mem_free_page(unsigned char page);
void mem_free_page_block(unsigned char page_block);

char *mem_get_user_buffer(void);
char *mem_copy_to_user_buffer(uintptr_t user_ptr, size_t count);
char *mem_copy_from_user_buffer(uintptr_t user_ptr, size_t count);

#endif
