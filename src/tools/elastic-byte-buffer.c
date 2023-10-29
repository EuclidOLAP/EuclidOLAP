#include <assert.h>
#include <unistd.h>
// #include <stdio.h>
// #include <string.h>
// #include <pthread.h>
// #include <semaphore.h>

// #include "log.h"
// #include "command.h"
#include "../utils.h"
#include "../memory-management.h"
// #include "cfg.h"
// #include "mdx.h"
// #include "mdd.h"
// #include "rb-tree.h"
#include "elastic-byte-buffer.h"

ByteBuf *buf__alloc(size_t capacity)
{
	assert(capacity > 0);

	ByteBuf *byte_buf = obj_alloc(sizeof(ByteBuf), OBJ_TYPE__ByteBuf);
	byte_buf->capacity = capacity;
	byte_buf->index = 0;
	byte_buf->buf_addr = allocate_memory(capacity);

	return byte_buf;
}

void buf_clear(ByteBuf *byte_buf)
{
	byte_buf->index = 0;
}

void buf_release(ByteBuf *byte_buf)
{
	// obj_release(byte_buf->buf_addr);
	release_memory(byte_buf->buf_addr);
	obj_release(byte_buf);
}

void *buf_cutting(ByteBuf *byte_buf, size_t count)
{
	if (byte_buf->capacity - byte_buf->index < count)
	{
		size_t added = count + byte_buf->index - byte_buf->capacity + 2048;
		byte_buf->capacity += added;
		byte_buf->buf_addr = realloc(byte_buf->buf_addr, byte_buf->capacity);
	}
	void *next_addr = ((char *)byte_buf->buf_addr) + byte_buf->index;
	byte_buf->index += count;
	return next_addr;
}

char *buf_cursor(ByteBuf *byte_buf)
{
	return buf_cutting(byte_buf, 0);
}

char *buf_starting(ByteBuf *byte_buf)
{
	return byte_buf->buf_addr;
}

size_t buf_size(ByteBuf *byte_buf)
{
	return byte_buf->index;
}

void buf_mv_cursor(ByteBuf *buf, ssize_t offset)
{
	buf->index += offset;
}

void buf_set_cursor(ByteBuf *buf, size_t position)
{
	buf->index = position;
}

void buf_append_short(ByteBuf *buf, short sval)
{
	*((short *)buf_cutting(buf, sizeof(short))) = sval;
}

void buf_append_int(ByteBuf *buf, int ival)
{
	*((int *)buf_cutting(buf, sizeof(int))) = ival;
}

void buf_append_long(ByteBuf *buf, long lval)
{
	*((long *)buf_cutting(buf, sizeof(long))) = lval;
}
