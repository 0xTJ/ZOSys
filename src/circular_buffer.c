#include "circular_buffer.h"

static inline size_t circular_buffer_next_head(struct circular_buffer *buffer) {
    size_t next_head = buffer->head + 1;
    if (next_head == buffer->size)
        next_head = 0;
    return next_head;
}

static inline size_t circular_buffer_next_tail(struct circular_buffer *buffer) {
    size_t next_tail = buffer->tail + 1;
    if (next_tail == buffer->size)
        next_tail = 0;
    return next_tail;
}

int circular_buffer_put(struct circular_buffer *buffer, unsigned char value) {
    size_t next_tail = circular_buffer_next_tail(buffer);
    if (next_tail == buffer->head) {
        // Buffer full
        return -1;
    }
    buffer->buffer[buffer->tail] = value;
    buffer->tail = next_tail;
    return value;
}

int circular_buffer_get(struct circular_buffer *buffer) {
    if (buffer->head == buffer->tail) {
        // Buffer empty
        return -1;
    }
    unsigned char value = buffer->buffer[buffer->head];
    buffer->head = circular_buffer_next_head(buffer);
    return value;
}
