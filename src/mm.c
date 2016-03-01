#include <system.h>

// Define max blocks allowed to be allocated
#define MAX_BLOCKS 2048

struct block;
struct freeblock;
typedef struct block block_t;
typedef struct freeblock freeblock_t;

struct block {
	char *base;
	block_t *next;
	u32 len;
};

struct freeblock {
	char *base;
	freeblock_t *next;
	u32 len;
};

// This is in start.asm at the end of the file (it's basically the same as the stack pointer + 4k;
extern u8* _sys_heap;
extern u8* end;

//static void *brkval;
u8 *heap_ptr;
u8 *free_ptr;
//static block_t *freelist;


// Initialize the Memory Manager
void init_mm()
{
	// Let's start simple and create just a standard heap
	heap_ptr = end + 1;
	heap_ptr[0] = 'H';
	heap_ptr[1] = 'E';
	heap_ptr[2] = 'A';
	heap_ptr[3] = 'P';
    heap_ptr[4] = '\0';
	free_ptr = heap_ptr  + 5;
}

void print_heap_magic()
{
    trace("heap magic");
    kprintf("Heap: %x | %x | %x", (u32)_sys_heap, (u32)heap_ptr, (u32)end);
}

// Byte Allocator for Heap
u8 * kmalloc(u32 nblks)
{
    if((u32)free_ptr + nblks > MAX_BLOCKS)
		return NULL;
	u8 *tmp = free_ptr;
	free_ptr = free_ptr + nblks + 1;
	return tmp;
}

/*
void * malloc(u32 size) 
 {
	// Find free block(s) for memory of size nbytes	
	// Set that block(s) to used
	// Possibly store some extra information
	// Return the address of this block
	return NULL;
}

void free(void * addr)
 {
	// Find block(s) starting at memory address of addr
	// Set these block(s) to free!
}
*/

