#ifndef JLIB_POOL_H
#define JLIB_POOL_H

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

/*
 * This is a chunked pool allocator. It's meant to allow you to allocate and
 * free items of the same size in roughly contiguous chunks of memory.
 */

typedef struct Pool Pool;
typedef struct Pool_free_node Pool_free_node;
typedef struct Pool_locator Pool_locator;

void pool_init(Pool *p, size_t item_size, size_t chunk_size, size_t inital_chunk_count);
void* pool_alloc(Pool *p);
void pool_free(Pool *p, void *ptr);
void pool_free_chunk(Pool *p, size_t index);
void pool_destroy(Pool *p);

#ifdef JLIB_POOL_IMPL

struct Pool {
	size_t item_size;
	size_t chunk_max_items;
	size_t chunks_in_use;
	size_t total_chunks;
	unsigned char **chunks;
	unsigned char *untouched_item;
	Pool_free_node *free;
};

struct Pool_free_node {
	Pool_free_node *next;
};

struct Pool_locator {
	uint32_t chunk, slot;
};

void pool_add_chunk(Pool *p) {
	Pool_free_node *free;

	if(p->chunks_in_use >= p->total_chunks) {
		p->total_chunks <<= 1;
		p->chunks = realloc(p->chunks, sizeof(unsigned char *) * p->total_chunks);
	}

	free = malloc(p->chunk_max_items * p->item_size);
	p->chunks[p->chunks_in_use] = (unsigned char*)free;
	++p->chunks_in_use;
	free->next = p->free;
}

void pool_init(Pool *p, size_t item_size, size_t chunk_max_items, size_t inital_chunk_count) {
	item_size = MAX(sizeof(Pool_free_node), item_size);
	p->item_size = item_size;
	p->chunk_max_items = chunk_max_items;
	p->total_chunks = inital_chunk_count;
	p->chunks_in_use = 1;
	p->chunks = malloc(sizeof(unsigned char*) * inital_chunk_count);
	void *ptr = malloc(chunk_max_items * item_size);
	p->free = NULL;
	p->untouched_item = ptr;
	p->chunks[0] = ptr;
}

void* pool_alloc(Pool *p) {
	void *ptr;

	if(p->free) {
		ptr = p->free;
		p->free = p->free->next;
	} else if(p->untouched_item) {
		ptr = p->untouched_item;
		p->untouched_item += p->item_size;
		if(p->untouched_item >= p->chunks[p->chunks_in_use-1] + p->chunk_max_items*p->item_size)
			p->untouched_item = NULL;
	} else {
		if(p->chunks_in_use >= p->total_chunks) {
			p->total_chunks <<= 1;
			p->chunks = realloc(p->chunks, sizeof(unsigned char *) * p->total_chunks);
		}
		ptr = p->chunks[p->chunks_in_use++] = malloc(p->chunk_max_items * p->item_size);
		p->untouched_item = (unsigned char*)ptr + p->item_size;
	}

	return ptr;
}

void pool_free(Pool *p, void *ptr) {
	unsigned char *free = ptr;
	size_t i;
	for(i = 0; i < p->chunks_in_use; ++i)
		if(free >= p->chunks[i] && free <= p->chunks[i] + p->chunk_max_items*p->item_size)
			break;
	if(i >= p->chunks_in_use)
		assert(0 && "pointer is not in pool bounds");
	Pool_free_node *free_node = ptr;
	free_node->next = p->free;
	p->free = free_node;
}

void pool_free_locator(Pool *p, Pool_locator l) {
	assert(0 && "pool_free_locator unimplemented");
}

void pool_free_chunk(Pool *p, size_t index) {
	assert(0 && "pool_free_chunk unimplemented");
}

void pool_destroy(Pool *p) {
	p->free = NULL;
	for(size_t i = 0; i < p->chunks_in_use; ++i)
		free(p->chunks[i]);
	free(p->chunks);
}

#endif

#endif
