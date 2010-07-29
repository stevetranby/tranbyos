#include <system.h>

// Define max blocks allowed to be allocated
#define MAX_BLOCKS	1024

struct block;
struct freeblock;
typedef struct block block_t;
typedef struct freeblock freeblock_t;

struct block {
	char *base;
	block_t *next;
	size_t len;
};

struct freeblock {
	char *base;
	freeblock_t *next;
	size_t len;
};

// This is in start.asm at the end of the file (it's basically the same as the stack pointer + 4k;
extern byte _sys_heap;
extern byte end;

//static void *brkval;
byte *heap_ptr;
byte *free_ptr;
//static block_t *freelist;



// Initialize the Memory Manager
void init_mm() {
	// Let's start simple and create just a standard heap
	heap_ptr = &end + 1;
	heap_ptr[0] = 'H';
	heap_ptr[1] = 'E';
	heap_ptr[2] = 'A';
	heap_ptr[3] = 'P';
	free_ptr = heap_ptr  + 4;
}

void print_heap_magic() {
	int i;
	printInt((int)&_sys_heap);
	putch('|');
	printInt((int)heap_ptr);
	putch('|');
	for(i = 0; i<4; ++i)
		putch(heap_ptr[i]);
	putch('\n');
}

byte * kmalloc(size_t nblks) {
	if((size_t)free_ptr + nblks > MAX_BLOCKS) 
		return NULL;
	byte *tmp = free_ptr;
	free_ptr = free_ptr + nblks + 1;
	return tmp;
}

/*
void * malloc(size_t size) {
	// Find free block(s) for memory of size nbytes	
	// Set that block(s) to used
	// Possibly store some extra information
	// Return the address of this block
	return NULL;
}

void free(void * addr) {
	// Find block(s) starting at memory address of addr
	// Set these block(s) to free!
}
*/

