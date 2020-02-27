#ifndef INCLUDE_MEM_H
#define INCLUDE_MEM_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#define USER_PTR(type) uintptr_t
#define MEM_USER_BUFFER_SIZE 0x100

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
void mem_free_page_block(unsigned char page);

USER_PTR(void) mem_memcpy_user_from_kconst(USER_PTR(void) dest, const void *src, size_t count);
USER_PTR(void) mem_memcpy_user_from_kdata(USER_PTR(void) dest, const void *src, size_t count);
USER_PTR(void) mem_memcpy_user_from_kstack(USER_PTR(void) dest, const void *src, size_t count);
USER_PTR(void) mem_memcpy_user_from_kernel(USER_PTR(void) dest, const void *src, size_t count);
void *mem_memcpy_kdata_from_user(void *dest, USER_PTR(const void) src, size_t count);
void *mem_memcpy_kstack_from_user(void *dest, USER_PTR(const void) src, size_t count);
void *mem_memcpy_kernel_from_user(void *dest, USER_PTR(const void) src, size_t count);
USER_PTR(void) mem_memcpy_user_from_user(USER_PTR(void) dest, USER_PTR(const void) src, size_t count);

void *mem_get_user_buffer(void);
void *mem_copy_to_user_buffer(USER_PTR(void) user_ptr, size_t count);
void *mem_copy_from_user_buffer(USER_PTR(void) user_ptr, size_t count);
int mem_user_strlen(USER_PTR(const char) user_ptr);  // Clobbers user buffer, returns -1 if the string is longer than MEM_USER_BUFFER_SIZE - 1
int mem_user_vector_len(USER_PTR(USER_PTR(void)) user_vector);   // Clobbers user buffer, return -1 if the vector table or any string is too long
bool mem_user_valid_ptr(USER_PTR(void) ptr);

#endif
