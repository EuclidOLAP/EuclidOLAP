#include <assert.h>
// #include <stdio.h>
// #include <string.h>
// #include <pthread.h>
// #include <semaphore.h>

// #include "log.h"
// #include "command.h"
#include "../utils.h"
// #include "cfg.h"
// #include "mdx.h"
// #include "mdd.h"
// #include "rb-tree.h"
#include "elastic-byte-buffer.h"

ByteBuf *buf__alloc(size_t capacity) {
	assert(capacity > 0);

	ByteBuf *byte_buf = obj_alloc(sizeof(ByteBuf), OBJ_TYPE__ByteBuf);
	byte_buf->capacity = capacity;
	byte_buf->index = 0;
	byte_buf->buf_addr = obj_alloc(capacity, OBJ_TYPE__RAW_BYTES);

	return byte_buf;
}

void buf_clear(ByteBuf *byte_buf) {
	byte_buf->index = 0;
}

void buf_release(ByteBuf *byte_buf) {
	obj_release(byte_buf->buf_addr);
	obj_release(byte_buf);
}

void *buf_cutting(ByteBuf *byte_buf, size_t count) {
	if (byte_buf->capacity - byte_buf->index < count) {
		size_t added = count + byte_buf->index - byte_buf->capacity + 2048;
		byte_buf->capacity += added;
		byte_buf->buf_addr = realloc(byte_buf->buf_addr, byte_buf->capacity);
	}
	void *next_addr = ((char *)byte_buf->buf_addr) + byte_buf->index;
	byte_buf->index += count;
	return next_addr;
}