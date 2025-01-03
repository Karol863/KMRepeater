#include <assert.h>

#include "config.h"

#define PAGE_SIZE 4096

typedef struct {
    INPUT input;
    LARGE_INTEGER time;
} EventInput;

typedef struct {
    EventInput *data;
    u64 offset;
    u64 capacity;
} Array;

BOOL ispoweroftwo(u64 x) {
	return (x & (x - 1)) == 0;
}

void array_init(Array *a) {
	assert(ispoweroftwo(RESERVED_MEMORY));

	a->data = VirtualAlloc(NULL, RESERVED_MEMORY * sizeof(EventInput), MEM_RESERVE, PAGE_NOACCESS);
	if (a->data == NULL) {
		fputs("Error: failed to reserve memory for the array!\n", stderr);
		exit(1);
	}

	a->data = VirtualAlloc(a->data, PAGE_SIZE * sizeof(EventInput), MEM_COMMIT, PAGE_READWRITE);
	if (a->data == NULL) {
		fputs("Error: failed to commit memory for the array!\n", stderr);
		exit(1);
	}
	a->offset = 0;
	a->capacity = PAGE_SIZE;
}

void array_alloc(Array *a, u64 size) {
	size = (size + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1);

	if (a->offset + size > RESERVED_MEMORY) {
		fputs("Error: array is full!\n", stderr);
		exit(1);
	}

	a->data = VirtualAlloc(a->data, size * sizeof(EventInput), MEM_COMMIT, PAGE_READWRITE);
	if (a->data == NULL) {
		fputs("Error: failed to commit memory for the array!\n", stderr);
		exit(1);
	}
}

void array_free(Array *a) {
	if (VirtualFree(a->data, 0, MEM_RELEASE) == 0) {
		fputs("Error: failed to free the array!\n", stderr);
		exit(1);
	}
	a->offset = 0;
	a->capacity = 0;
}
