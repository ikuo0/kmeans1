
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mempool.h"
#include "../exception/exception.h"
#include "../format/format.h"

static memory_block inuse_begin = {0, 0, NULL, NULL, NULL};
static memory_block inuse_end = {0, 0, NULL, NULL, NULL};
static memory_block unuse_begin = {0, 0, NULL, NULL, NULL};
static memory_block unuse_end = {0, 0, NULL, NULL, NULL};

static int _mempool_alloc_count = 0;
static int _mempool_free_count = 0;
static int _mempool_recycle_count = 0;

void mempool_insert_block(memory_block* point, memory_block *block) {
	point->prev->next = block;
	block->prev = point->prev;
	block->next = point;
	point->prev = block;
}

memory_block* mempool_find_block(memory_block *block, char *p) {
	memory_block* iter = block;
	for(; iter != NULL; iter = iter->next) {
		if(iter->mem == p) {
			return iter;
		}
	}
	return NULL;
}

memory_block* mempool_find_inuse_block(char *p) {
	return mempool_find_block(&inuse_begin, p);
}

memory_block* mempool_find_unuse_block(char *p) {
	return mempool_find_block(&unuse_begin, p);
}

void mempool_remove_block(memory_block* block) {
	block->next->prev = block->prev;
	block->prev->next = block->next;
}

void memory_inuse_append(memory_block* block) {
	mempool_insert_block(&inuse_end, block);
}

void memory_unuse_append(memory_block* block) {
	mempool_insert_block(&unuse_end, block);
}

double mempool_userate(mempool_instance* self) {
    return (double)self->position / (double)self->size;
}

// 初期化関数
void mempool_init(mempool_instance* self, size_t size) {
	printf("%s reserve size: %ld\n", __FUNCTION__, size);
	self->size = size;
	self->pool = malloc(self->size);
	if(self->pool == NULL) {
		exception_jump(format("malloc failed, request size=%ld", size), 9);
	}
	self->position = 0;

	inuse_begin.next = &inuse_end;
	inuse_end.prev = &inuse_begin;
	unuse_begin.next = &unuse_end;
	unuse_end.prev = &unuse_begin;
	self->inuse = &inuse_begin;
	self->unuse = &unuse_begin;
}

// 終了関数
void mempool_end(mempool_instance* self) {
	printf("%s\n", __FUNCTION__);
	printf("alloc count: %d\n", _mempool_alloc_count);
	printf("free count: %d\n", _mempool_free_count);
	printf("recycle count: %d\n", _mempool_recycle_count);
	printf("userate: %lf\n", mempool_userate(self));
	free(self->pool);
}

// メモリ空き検索
memory_block* mempool_find_unuse(mempool_instance* self, size_t size) {
	//printf("%s\n", __FUNCTION__);
	memory_block* iter;
	for(iter = self->unuse; iter != NULL; iter = iter->next) {
		if (iter->size >= size) {
			return iter;
		}
	}
	return NULL;
}

size_t mempool_margin_size(size_t size) {
	//log2l
	return 1l;
}

// メモリ確保
char* mempool_alloc(mempool_instance* self, size_t size) {
	_mempool_alloc_count++;
	//printf("%s, request size: %ld, use rate=%lf\n", __FUNCTION__, size, mempool_userate(self));
	//size_t required_size = mempool_margin_size(size);
	memory_block* iter = mempool_find_unuse(self, size);
	if(iter != NULL) {
		_mempool_recycle_count++;
        //printf("recycle %p\n", iter);
        mempool_remove_block(iter);
        memory_inuse_append(iter);
		return iter->mem;
	} else {
		memory_block* block;
		size_t required_size = size + sizeof(memory_block);
		if((self->size - self->position) < required_size) {
			exception_jump(format("alloc from pool failed, request size=%ld", size), 9);
			return NULL;
		} else {
			block = (memory_block*)(self->pool + self->position);
			block->position = self->position + sizeof(memory_block);
			block->size = size;
			block->mem = self->pool + block->position;
			self->position = self->position + required_size;
			memory_inuse_append(block);
            //printf("new block %p\n", block);
			return block->mem;
		}
	}
}

// メモリ空き検索
memory_block* mempool_find_inuse(mempool_instance* self, memory_block* block) {
	memory_block* iter;
	for(iter = self->inuse; iter != NULL; iter = iter->next) {
		if(iter->next == block) {
			iter->next = block->next;
		}
	}
	return NULL;
}

void mempool_free(mempool_instance* self, char* p) {
	_mempool_free_count++;
	memory_block* block = mempool_find_inuse_block(p);
    if(block == NULL) {
        return;
    }
	mempool_remove_block(block);
	memory_unuse_append(block);
}

#ifdef MEMPOOL_UNITTEST

int main(int argc, char** argv) {
	mempool_instance instance;
	mempool_init(&instance, 1024 * 1024 * 1024);

	printf("%lf\n", log2l(250));
	exit(1);

	char* a = mempool_alloc(&instance, 100);
	char* b = mempool_alloc(&instance, 100);
	char* c = mempool_alloc(&instance, 100);
	strcpy(a, "aaaaaaaaaaaaaaaaaaaaaaaaaa");
	strcpy(b, "dfgsfeghsfedhgsethgset");
	strcpy(c, "ppppppppppppppppppppppppppppppp");
	printf("a: %p, b: %p, c: %p\n", a, b, c);
	printf("%s\n", a);
	printf("%s\n", b);
	printf("%s\n", c);

	mempool_free(&instance, a);

	char* d = mempool_alloc(&instance, 100);
	printf("d: %p\n", d);

	mempool_end(&instance);
	return 0;
}

#endif

