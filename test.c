#define JLIB_POOL_IMPL
#define JLIB_BASIC_IMPL
#define JLIB_ARENA_IMPL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arena.h"
#include "basic.h"
#include "pool.h"

#define STATICARRLEN(x) (sizeof(x)/sizeof(*x))
#define STRLEN(s) (STATICARRLEN(s)-1)

typedef struct Node Node;
typedef struct List List;

struct Node {
	int i;
	Node *next;
};

struct List {
	Node *head;
	Node *tail;
	Pool nodepool;
};

void list_init(List *l) {
	pool_init(&l->nodepool, sizeof(Node), 5, 1);
	l->head = l->tail = NULL;
}

void list_append(List *l, int i) {
	Node *tail = l->tail;
	if(tail == NULL) {
		l->head = l->tail = pool_alloc(&l->nodepool);
	} else {
		l->tail->next = pool_alloc(&l->nodepool);
		l->tail = l->tail->next;
	}
	l->tail->i = i;
	l->tail->next = NULL;
}

void list_print(List *l) {
	for(Node *np = l->head; np; np = np->next)
		printf("NODE\ni: %i\n", np->i);
}

Node* list_pop(List *l) {
	Node *np = NULL, *popped;
	popped = l->tail;
	for(np = l->head; np->next != l->tail; np = np->next);
	np->next = NULL;
	l->tail = np;
	return popped;
}

void list_destroy(List *l) {
	pool_destroy(&l->nodepool);
	l->head = l->tail = NULL;
}

// TODO make unit tests for libs
int main() {
	unsigned char buf[152];
	int ibuf[] = {
		35, 2, 1, 40, 31, 23, 27, 15, 25, 5, 6, 41, 29,
		14, 4, 34, 20, 16, 11, 30, 19, 36, 39, 26, 32,
		42, 18, 13,
	};
	size_t ni = STATICARRLEN(ibuf);
	Arena arena = {0};
	arena_init(&arena, buf, STATICARRLEN(buf));

	char *cp = arena_alloc(&arena, STATICARRLEN("hello world\n"));
	strcpy(cp, "hello world\n");
	int *ip = arena_alloc(&arena, sizeof(int) * ni);
	for(int i = 0; i < ni; ++i) ip[i] = ibuf[i];
	printf("cp = %s",cp);
	for(int i = 0; i < ni; ++i) printf("ip[%i] = %i\n",i,ip[i]);
	size_t cp_len = strlen(cp) + 1;
	cp = arena_resize(&arena, cp, cp_len, cp_len + STRLEN("I'm alive!\n"));
	assert(cp && "out of memory");
	strcpy(cp+cp_len-1, "I'm alive!\n");
	printf("cp = %s",cp);
	printf("used %zu bytes\n", arena.cur_offset);
	arena_free(&arena);

	List list = {0};
	list_init(&list);
	for(int i = 0; i < ni; ++i) list_append(&list, ibuf[i]);
	list_print(&list);
	printf("\n##############################\n");
	for(int i = 0; i < (ni>>1); ++i)
		pool_free(&list.nodepool, list_pop(&list));
	list_print(&list);
	printf("\n########### POOL FREE LIST ###########\n");
	size_t x = 0;
	for(Pool_free_node *free = list.nodepool.free; free; free = free->next, ++x)
		printf("FREE ITEM %p\n", (void*)free);
	assert(x == ni>>1);
	printf("ni = %zu\n",ni);
	printf("UNTOUCHED ITEM %p\n", list.nodepool.untouched_item);
	list_destroy(&list);
}
