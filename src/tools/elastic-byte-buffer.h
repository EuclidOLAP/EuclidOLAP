#ifndef ELASTIC_BYTEBUF_H
#define ELASTIC_BYTEBUF_H 1

#include <stdio.h>

typedef struct elastic_byte_buf {
	void *buf_addr;
	size_t capacity;
	size_t index;
} ByteBuf;

ByteBuf *buf__alloc(size_t capacity);

void buf_clear(ByteBuf *byte_buf);

void buf_release(ByteBuf *byte_buf);

char *buf_cursor(ByteBuf *byte_buf);

/**
 * Returns the address of a memory buffer with enough space for the data to be stored in it.
 */
void *buf_cutting(ByteBuf *byte_buf, size_t count);

char *buf_starting(ByteBuf *byte_buf);

size_t buf_size(ByteBuf *byte_buf);

#endif