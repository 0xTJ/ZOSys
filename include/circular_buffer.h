#ifndef INCLUDE_CIRCULAR_BUFFER_H
#define INCLUDE_CIRCULAR_BUFFER_H

#include <stddef.h>

struct circular_buffer {
    unsigned char *buffer;
    size_t size;
    size_t head;
    size_t tail;
};

int circular_buffer_put(struct circular_buffer *buffer, unsigned char value);
int circular_buffer_get(struct circular_buffer *buffer);

#endif
