
#ifndef _MEMPOOL_H_
#define _MEMPOOL_H_

#include <stddef.h>

typedef struct _memory_block {
	size_t position;
	size_t size;
	char *mem;
	struct _memory_block* prev;
	struct _memory_block* next;
} memory_block;

typedef struct _mempool_instance {
	size_t size;
	char* pool;
	size_t position;

	// 使用中ブロック
	memory_block *inuse;

	// 未使用ブロック
	memory_block *unuse;
} mempool_instance;

extern void mempool_init(mempool_instance* self, size_t size);
extern void mempool_end(mempool_instance* self);
extern char* mempool_alloc(mempool_instance* self, size_t size);
extern void mempool_free(mempool_instance* self, char* p);

#endif
