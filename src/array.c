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

static inline BOOL ispoweroftwo(u64 x) {
	return (x & (x - 1)) == 0;
}

void array_init(Array *a) {
	if (!ispoweroftwo(RESERVED_MEMORY)) {
		fputs("Error: reserved memory MUST be a power of 2.\n", stderr);
		exit(1);
	}

	a->data = VirtualAlloc(NULL, RESERVED_MEMORY, MEM_RESERVE, PAGE_NOACCESS);
	if (a->data == NULL) {
		fputs("Error: failed to reserve memory for the array.\n", stderr);
		exit(1);
	}

	a->data = VirtualAlloc(a->data, PAGE_SIZE * sizeof(EventInput), MEM_COMMIT, PAGE_READWRITE);
	if (a->data == NULL) {
		fputs("Error: failed to commit memory for the array.\n", stderr);
		exit(1);
	}
	a->offset = 0;
	a->capacity = PAGE_SIZE;
}

void array_alloc(Array *a, u64 size) {
	size = (size + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1);

	if (size * sizeof(EventInput) + a->offset > RESERVED_MEMORY) {
		fputs("Error: array is full.\n", stderr);
		exit(1);
	}

	a->data = VirtualAlloc(a->data, size * sizeof(EventInput), MEM_COMMIT, PAGE_READWRITE);
	if (a->data == NULL) {
		fputs("Error: failed to commit memory for the array.\n", stderr);
		exit(1);
	}
}
