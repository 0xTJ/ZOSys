#ifndef INCLUDE_CIRCULAR_BUFFER_H
#define INCLUDE_CIRCULAR_BUFFER_H

#include <stdbool.h>
#include <stddef.h>

struct circular_buffer {
    unsigned char *buffer;
    size_t size;
    size_t head;
    size_t tail;
};

int circular_buffer_put(struct circular_buffer *buffer, unsigned char value);
int circular_buffer_get(struct circular_buffer *buffer) __z88dk_fastcall;
bool circular_buffer_is_full(struct circular_buffer *buffer) __z88dk_fastcall;
size_t circular_buffer_space_left(struct circular_buffer *buffer) __z88dk_fastcall;

#endif
