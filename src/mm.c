#include <system.h>

// NOTE: the linker symbol variables have their addresses defined
//       value stored as the pointer is data found at symbol address
// ex: 0x00100000 (address of _text_start)
//     0x1badb002 (data @ _text_start cast as void*) <- multiboot header
extern intptr_t _text_start;
extern intptr_t _text_end;
extern intptr_t _data_start;
extern intptr_t _data_end;
extern intptr_t _bss_start;
extern intptr_t _bss_end;
// these
extern intptr_t sys_stack_bottom;
extern intptr_t sys_stack_top;
extern intptr_t sys_heap_bottom;
extern intptr_t sys_heap_top;

////////////////////////////////////////////////////////////////////////////////


i8 standard_lessthan_predicate(type_t a, type_t b)
{
    return (a<b)?1:0;
}

ordered_array_t create_ordered_array(u32 max_size, lessthan_predicate_t less_than)
{
    ordered_array_t to_ret;
    to_ret.array = (void*)kmalloc(max_size*sizeof(type_t));
    kmemset(to_ret.array, 0, max_size*sizeof(type_t));
    to_ret.size = 0;
    to_ret.max_size = max_size;
    to_ret.less_than = less_than;
    return to_ret;
}

ordered_array_t place_ordered_array(void *addr, u32 max_size, lessthan_predicate_t less_than)
{
    ordered_array_t to_ret;
    to_ret.array = (type_t*)addr;
    kmemset(to_ret.array, 0, max_size*sizeof(type_t));
    to_ret.size = 0;
    to_ret.max_size = max_size;
    to_ret.less_than = less_than;
    return to_ret;
}

void destroy_ordered_array(ordered_array_t *array)
{
    //    kfree(array->array);
}

void insert_ordered_array(type_t item, ordered_array_t *array)
{
    ASSERT(array->less_than, "");
    u32 iterator = 0;
    while (iterator < array->size && array->less_than(array->array[iterator], item))
        iterator++;
    if (iterator == array->size) // just add at the end of the array.
        array->array[array->size++] = item;
    else
    {
        type_t tmp = array->array[iterator];
        array->array[iterator] = item;
        while (iterator < array->size)
        {
            iterator++;
            type_t tmp2 = array->array[iterator];
            array->array[iterator] = tmp;
            tmp = tmp2;
        }
        array->size++;
    }
}

type_t lookup_ordered_array(u32 i, ordered_array_t *array)
{
    ASSERT(i < array->size, "");
    return array->array[i];
}

void remove_ordered_array(u32 i, ordered_array_t *array)
{
    while (i < array->size)
    {
        array->array[i] = array->array[i+1];
        i++;
    }
    array->size--;
}


////////////////////////////////////////////////////////////////////////////////////

// The kernel's page directory
page_directory * kernel_directory = 0;

// The current page directory;
page_directory * current_directory = 0;

// A bitset of frames - used or free.
u32 *frames;
u32 nframes;

// Defined in kheap.c
extern u32 placement_address;

// Macros used in the bitset algorithms.
#define INDEX_FROM_BIT(a) (a/(8*4))
#define OFFSET_FROM_BIT(a) (a%(8*4))

// Static function to set a bit in the frames bitset
static void set_frame(u32 frame_addr)
{
    u32 frame = frame_addr/0x1000;
    u32 idx = INDEX_FROM_BIT(frame);
    u32 off = OFFSET_FROM_BIT(frame);
    frames[idx] |= (0x1 << off);
}

// Static function to clear a bit in the frames bitset
static void clear_frame(u32 frame_addr)
{
    u32 frame = frame_addr/0x1000;
    u32 idx = INDEX_FROM_BIT(frame);
    u32 off = OFFSET_FROM_BIT(frame);
    frames[idx] &= ~(0x1 << off);
}

// Static function to test if a bit is set.
static u32 test_frame(u32 frame_addr)
{
    u32 frame = frame_addr/0x1000;
    u32 idx = INDEX_FROM_BIT(frame);
    u32 off = OFFSET_FROM_BIT(frame);
    return (frames[idx] & (0x1 << off));
}

// Static function to find the first free frame.
static u32 first_frame()
{
    u32 i, j;
    for (i = 0; i < INDEX_FROM_BIT(nframes); i++)
    {
        if (frames[i] != 0xFFFFFFFF) // nothing free, exit early.
        {
            // at least one bit is free here.
            for (j = 0; j < 32; j++)
            {
                u32 toTest = 0x1 << j;
                if ( !(frames[i]&toTest) )
                {
                    return i*4*8+j;
                }
            }
        }
    }
    return 0;
}
// Function to allocate a frame.
void alloc_frame(page_table_entry* page, int kernel, int writeable)
{
    if (page->frameAddress != 0)
    {
        return; // Frame was already allocated, return straight away.
    }
    else
    {
        u32 idx = first_frame(); // idx is now the index of the first free frame.
        if (idx == (u32)-1)
        {
            // PANIC is just a macro that prints a message to the screen then hits an infinite loop.
            //TODO: PANIC("No free frames!");
            trace("no free frames!");
        }
        set_frame(idx * 0x1000); // this frame is now ours!
        page->present = 1; // Mark it as present.
        page->readwrite = writeable ? 1 : 0;
        page->accessRing3 = kernel ? 0 : 1;
        page->frameAddress = idx;
    }
}

// Function to deallocate a frame.
void free_frame(page_table_entry* page)
{
    u32 frame;
    if (!(frame = page->frameAddress))
    {
        return; // The given page didn't actually have an allocated frame!
    }
    else
    {
        clear_frame(frame); // Frame is now free again.
        page->frameAddress = 0x0; // Page now doesn't have a frame.
    }
}





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

// NOTE: current kernel blocks are byte-sized
u32 blocks_used[MAX_BLOCKS];

//static void *brkval;
u8* heap_ptr;
u32 free_ptr;
//static block_t *freelist;

// Initialize the Memory Manager
void init_mm()
{
    kmemset(blocks_used, 0, MAX_BLOCKS);
    // Let's start simple and create just a standard heap
    heap_ptr = (u8*)&sys_heap_bottom + 1;
    free_ptr = (u32)heap_ptr;

    trace("heap_ptr = %x\n", heap_ptr);
    trace("free_ptr = %x\n", free_ptr);
    {
        // TEST MAGIC
        u8* ptr = kmalloc_b(5);
        trace("ptr = %p\n", ptr);
        ptr[0] = 'H';
        ptr[1] = 'E';
        ptr[2] = 'A';
        ptr[3] = 'P';
        ptr[4] = '\0';
    }
    {
        // TEST MAGIC
        u8* ptr = kmalloc_b(5);
        trace("ptr = %p\n", ptr);
        ptr[0] = 'H';
        ptr[1] = 'E';
        ptr[2] = 'A';
        ptr[3] = 'P';
        ptr[4] = '\0';
    }
}

void print_heap_magic()
{
    trace("heap magic\n");

    // NOTE: the linker symbol variables have their addresses defined
    //       value stored as the pointer is data found at symbol address
    // ex: 0x00100000 (address of _text_start)
    //     0x1badb002 (data @ _text_start cast as void*) <- multiboot header
    trace("Text Section: %p | %p\n", &_text_start, &_text_end);
    trace("Data Section: %p | %p\n", &_data_start, &_data_end);
    trace("BSS Section:  %p | %p\n", &_bss_start, &_bss_end);
    trace("STACK:        %p | %p\n", &sys_stack_bottom, &sys_stack_top);
    trace("HEAP:         %p | %p\n", &sys_heap_bottom, &sys_heap_top);
    trace("Heap Magic String: %p '%s'\n", heap_ptr, heap_ptr);
    trace("Free Ptr:          %p\n", free_ptr);

}

void print_heap_bytes(u32 n)
{
    trace("print_heap_bytes\n");
    int i = 0;
    for(u32* ptr = (u32*)heap_ptr; i < n; ++i, ++ptr) {
        trace("%x", *ptr);
        if(0 == (i+1) % 8) trace("\n");
        else trace(" ");
    }

}

// TODO: a lot of potential places to macro or pre-proc out the excess
// TODO: do we want this? it's a nice handy shortcut
#define FOR(n) for(int i=0; i<10;)

void print_blocks_avail()
{
//    FOR(MAX_BLOCKS) {
//        trace("", block_avail);
//    }
    for(int i = 0; i < MAX_BLOCKS; ++i)
    {
        trace("%d", blocks_used[i]);
        if(0 == (i+1) % 60) trace("\n");
        else trace(",");
    }
}

// TODO: allocate extra block on each side and fill with debug markers
// TODO: make block size larger (multiple sizes)
// Byte Allocator for Heap
u8* kmalloc_b(u32 nblks)
{
    return (u8*)kmalloc(nblks);

//    u32 offset = free_ptr - heap_ptr;
//
//    // TODO: wrap around search and try to find contiguous space in freed blocks
//    //       if wrapping need to check blocks_used[] to make sure it's free
//    if((u32)offset + nblks > MAX_BLOCKS) {
//        trace("TODO: need to have wrap around malloc\n");
//		return NULL;
//    }
//
//    // mark used
//    trace("blocks_used %p, %d, %d, %d\n", blocks_used + offset, offset, nblks, nblks);
//    kmemset(blocks_used + offset, nblks, nblks);
//
//    // return init block (debug fill with known data)
//    u8* tmp = free_ptr;
//	free_ptr = free_ptr + nblks;
//#ifdef _DEBUG_
//    kmemset(tmp, 0xee, nblks);
//#endif
//    trace("malloc ret ptr = %p\n", tmp);
//	return tmp;
}

u32 kmalloc_int(u32 sz, int align, u32 *phys)
{
    // This will eventually call malloc() on the kernel heap.
    // For now, though, we just assign memory at placement_address
    // and increment it by sz. Even when we've coded our kernel
    // heap, this will be useful for use before the heap is initialised.

    if (align == 1 && (free_ptr & 0xFFFFF000) )
    {
        // Align the placement address;
        free_ptr &= 0xFFFFF000;
        free_ptr += 0x1000;
    }

    if (phys)
    {
        *phys = free_ptr;
    }

    u32 tmp = free_ptr;
    free_ptr += sz;
    return tmp;
}

u32 kmalloc_a(u32 sz)
{
    return kmalloc_int(sz, 1, 0);
}

u32 kmalloc_p(u32 sz, u32 *phys)
{
    return kmalloc_int(sz, 0, phys);
}

u32 kmalloc_ap(u32 sz, u32 *phys)
{
    return kmalloc_int(sz, 1, phys);
}

u32 kmalloc(u32 sz)
{
    return kmalloc_int(sz, 0, 0);
}

static void expand(u32 new_size, heap_t *heap)
{
    // Sanity check.
    //TODO: ASSERT(new_size > heap->end_address - heap->start_address);

    // Get the nearest following page boundary.
    if ((new_size & 0xFFFFF000) != 0)
    {
        new_size &= 0xFFFFF000;
        new_size += 0x1000;
    }

    // Make sure we are not overreaching ourselves.
    //TODO: ASSERT(heap->start_address+new_size <= heap->max_address);

    // This should always be on a page boundary.
    u32 old_size = heap->end_address-heap->start_address;

    u32 i = old_size;
    while (i < new_size)
    {
        alloc_frame( get_page(heap->start_address+i, 1, kernel_directory),
                    (heap->supervisor)?1:0, (heap->readonly)?0:1);
        i += 0x1000 /* page size */;
    }
    heap->end_address = heap->start_address+new_size;
}

static u32 contract(u32 new_size, heap_t *heap)
{
    // Sanity check.
    //TODO: ASSERT(new_size < heap->end_address-heap->start_address);

    // Get the nearest following page boundary.
    if (new_size&0x1000)
    {
        new_size &= 0x1000;
        new_size += 0x1000;
    }

    // Don't contract too far!
    if (new_size < HEAP_MIN_SIZE)
        new_size = HEAP_MIN_SIZE;

    u32 old_size = heap->end_address-heap->start_address;
    u32 i = old_size - 0x1000;
    while (new_size < i)
    {
        free_frame(get_page(heap->start_address+i, 0, kernel_directory));
        i -= 0x1000;
    }

    heap->end_address = heap->start_address + new_size;
    return new_size;
}

static i32 find_smallest_hole(u32 size, u8 page_align, heap_t* heap)
{
    // Find the smallest hole that will fit.
    u32 iterator = 0;
    while (iterator < heap->index.size)
    {
        header_t *header = (header_t*)lookup_ordered_array(iterator, &heap->index);
        // If the user has requested the memory be page-aligned
        if (page_align > 0)
        {
            // Page-align the starting point of this header.
            u32 location = (u32)header;
            i32 offset = 0;
            if (((location + sizeof(header_t)) & 0xFFFFF000) != 0)
                offset = 0x1000 /* page size */  - (location+sizeof(header_t))%0x1000;
            i32 hole_size = (i32)header->size - offset;
            // Can we fit now?
            if (hole_size >= (i32)size)
                break;
        }
        else if (header->size >= size)
            break;
        iterator++;
    }
    // Why did the loop exit?
    if (iterator == heap->index.size)
        return -1; // We got to the end and didn't find anything.
    else
        return iterator;
}

static i8 header_t_less_than(void*a, void *b)
{
    return (((header_t*)a)->size < ((header_t*)b)->size)?1:0;
}

heap_t *create_heap(u32 start, u32 end_addr, u32 max, u8 supervisor, u8 readonly)
{
    trace("creating heap: ");

    heap_t *heap = (heap_t*)kmalloc(sizeof(heap_t));

    // All our assumptions are made on startAddress and endAddress being page-aligned.
    ASSERT(start % 0x1000 == 0, "");
    ASSERT(end_addr % 0x1000 == 0, "");

    // Initialise the index.
    heap->index = place_ordered_array( (void*)start, HEAP_INDEX_SIZE, &header_t_less_than);

    // Shift the start address forward to resemble where we can start putting data.
    start += sizeof(type_t)*HEAP_INDEX_SIZE;

    // Make sure the start address is page-aligned.
    if ((start & 0xFFFFF000) != 0)
    {
        start &= 0xFFFFF000;
        start += 0x1000;
    }
    // Write the start, end and max addresses into the heap structure.
    heap->start_address = start;
    heap->end_address = end_addr;
    heap->max_address = max;
    heap->supervisor = supervisor;
    heap->readonly = readonly;

    // We start off with one large hole in the index.
    header_t *hole = (header_t *)start;
    hole->size = end_addr-start;
    hole->magic = HEAP_MAGIC;
    hole->is_hole = 1;
    insert_ordered_array((void*)hole, &heap->index);

    return heap;
}

void* alloc(u32 size, u8 page_align, heap_t* heap)
{
    trace("alloc: ");

    // Make sure we take the size of header/footer into account.
    u32 new_size = size + sizeof(header_t) + sizeof(footer_t);
    // Find the smallest hole that will fit.
    i32 iterator = find_smallest_hole(new_size, page_align, heap);

    if (iterator == -1) // If we didn't find a suitable hole
    {
        // Save some previous data.
        u32 old_length = heap->end_address - heap->start_address;
        u32 old_end_address = heap->end_address;

        // We need to allocate some more space.
        expand(old_length+new_size, heap);
        u32 new_length = heap->end_address-heap->start_address;

        // Find the endmost header. (Not endmost in size, but in location).
        iterator = 0;
        // Vars to hold the index of, and value of, the endmost header found so far.
        u32 idx = -1; u32 value = 0x0;
        while (iterator < heap->index.size)
        {
            u32 tmp = (u32)lookup_ordered_array(iterator, &heap->index);
            if (tmp > value)
            {
                value = tmp;
                idx = iterator;
            }
            iterator++;
        }

        // If we didn't find ANY headers, we need to add one.
        if (idx == -1)
        {
            header_t *header = (header_t *)old_end_address;
            header->magic = HEAP_MAGIC;
            header->size = new_length - old_length;
            header->is_hole = 1;
            footer_t *footer = (footer_t *) (old_end_address + header->size - sizeof(footer_t));
            footer->magic = HEAP_MAGIC;
            footer->header = header;
            insert_ordered_array((void*)header, &heap->index);
        }
        else
        {
            // The last header needs adjusting.
            header_t *header = lookup_ordered_array(idx, &heap->index);
            header->size += new_length - old_length;
            // Rewrite the footer.
            footer_t *footer = (footer_t *) ( (u32)header + header->size - sizeof(footer_t) );
            footer->header = header;
            footer->magic = HEAP_MAGIC;
        }
        // We now have enough space. Recurse, and call the function again.
        return alloc(size, page_align, heap);
    }

    header_t *orig_hole_header = (header_t *)lookup_ordered_array(iterator, &heap->index);
    u32 orig_hole_pos = (u32)orig_hole_header;
    u32 orig_hole_size = orig_hole_header->size;
    // Here we work out if we should split the hole we found into two parts.
    // Is the original hole size - requested hole size less than the overhead for adding a new hole?
    if (orig_hole_size-new_size < sizeof(header_t)+sizeof(footer_t))
    {
        // Then just increase the requested size to the size of the hole we found.
        size += orig_hole_size-new_size;
        new_size = orig_hole_size;
    }

    // If we need to page-align the data, do it now and make a new hole in front of our block.
    if (page_align && orig_hole_pos&0xFFFFF000)
    {
        u32 new_location   = orig_hole_pos + 0x1000 /* page size */ - (orig_hole_pos&0xFFF) - sizeof(header_t);
        header_t *hole_header = (header_t *)orig_hole_pos;
        hole_header->size     = 0x1000 /* page size */ - (orig_hole_pos&0xFFF) - sizeof(header_t);
        hole_header->magic    = HEAP_MAGIC;
        hole_header->is_hole  = 1;
        footer_t *hole_footer = (footer_t *) ( (u32)new_location - sizeof(footer_t) );
        hole_footer->magic    = HEAP_MAGIC;
        hole_footer->header   = hole_header;
        orig_hole_pos         = new_location;
        orig_hole_size        = orig_hole_size - hole_header->size;
    }
    else
    {
        // Else we don't need this hole any more, delete it from the index.
        remove_ordered_array(iterator, &heap->index);
    }

    // Overwrite the original header...
    header_t *block_header  = (header_t *)orig_hole_pos;
    block_header->magic     = HEAP_MAGIC;
    block_header->is_hole   = 0;
    block_header->size      = new_size;
    // ...And the footer
    footer_t *block_footer  = (footer_t *) (orig_hole_pos + sizeof(header_t) + size);
    block_footer->magic     = HEAP_MAGIC;
    block_footer->header    = block_header;

    // We may need to write a new hole after the allocated block.
    // We do this only if the new hole would have positive size...
    if (orig_hole_size - new_size > 0)
    {
        header_t *hole_header = (header_t *) (orig_hole_pos + sizeof(header_t) + size + sizeof(footer_t));
        hole_header->magic    = HEAP_MAGIC;
        hole_header->is_hole  = 1;
        hole_header->size     = orig_hole_size - new_size;
        footer_t *hole_footer = (footer_t *) ( (u32)hole_header + orig_hole_size - new_size - sizeof(footer_t) );
        if ((u32)hole_footer < heap->end_address)
        {
            hole_footer->magic = HEAP_MAGIC;
            hole_footer->header = hole_header;
        }
        // Put the new hole in the index;
        insert_ordered_array((void*)hole_header, &heap->index);
    }

    // ...And we're done!
    return (void *) ( (u32)block_header+sizeof(header_t) );
}

void free(void *p, heap_t *heap)
{
    // Exit gracefully for null pointers.
    if (p == 0)
        return;

    // Get the header and footer associated with this pointer.
    header_t *header = (header_t*) ( (u32)p - sizeof(header_t) );
    footer_t *footer = (footer_t*) ( (u32)header + header->size - sizeof(footer_t) );

    // Sanity checks.
    ASSERT(header->magic == HEAP_MAGIC, "");
    ASSERT(footer->magic == HEAP_MAGIC, "");

    // Make us a hole.
    header->is_hole = 1;

    // Do we want to add this header into the 'free holes' index?
    char do_add = 1;

    // Unify left
    // If the thing immediately to the left of us is a footer...
    footer_t *test_footer = (footer_t*) ( (u32)header - sizeof(footer_t) );
    if (test_footer->magic == HEAP_MAGIC &&
        test_footer->header->is_hole == 1)
    {
        u32 cache_size = header->size; // Cache our current size.
        header = test_footer->header;     // Rewrite our header with the new one.
        footer->header = header;          // Rewrite our footer to point to the new header.
        header->size += cache_size;       // Change the size.
        do_add = 0;                       // Since this header is already in the index, we don't want to add it again.
    }

    // Unify right
    // If the thing immediately to the right of us is a header...
    header_t *test_header = (header_t*) ( (u32)footer + sizeof(footer_t) );
    if (test_header->magic == HEAP_MAGIC &&
        test_header->is_hole)
    {
        header->size += test_header->size; // Increase our size.
        test_footer = (footer_t*) ( (u32)test_header + // Rewrite it's footer to point to our header.
                                   test_header->size - sizeof(footer_t) );
        footer = test_footer;
        // Find and remove this header from the index.
        u32 iterator = 0;
        while ( (iterator < heap->index.size) &&
               (lookup_ordered_array(iterator, &heap->index) != (void*)test_header) )
            iterator++;

        // Make sure we actually found the item.
        ASSERT(iterator < heap->index.size, "");
        // Remove it.
        remove_ordered_array(iterator, &heap->index);
    }

    // If the footer location is the end address, we can contract.
    if ( (u32)footer+sizeof(footer_t) == heap->end_address)
    {
        u32 old_length = heap->end_address-heap->start_address;
        u32 new_length = contract( (u32)header - heap->start_address, heap);
        // Check how big we will be after resizing.
        if (header->size - (old_length-new_length) > 0)
        {
            // We will still exist, so resize us.
            header->size -= old_length-new_length;
            footer = (footer_t*) ( (u32)header + header->size - sizeof(footer_t) );
            footer->magic = HEAP_MAGIC;
            footer->header = header;
        }
        else
        {
            // We will no longer exist :(. Remove us from the index.
            u32 iterator = 0;
            while ( (iterator < heap->index.size) &&
                   (lookup_ordered_array(iterator, &heap->index) != (void*)test_header) )
                iterator++;
            // If we didn't find ourselves, we have nothing to remove.
            if (iterator < heap->index.size)
                remove_ordered_array(iterator, &heap->index);
        }
    }
    
    // If required, add us to the index.
    if (do_add == 1)
        insert_ordered_array((void*)header, &heap->index);
    
}

// TODO: everything below needs to be studied once working, then re-written by hand

//////////////////////////////////////////////////
// PAE Paging
//
//i64 page_dir_ptr_tab[4] __attribute__((aligned(0x20)));
//
//void init_page_directory_PAE()
//{
//    page_dir_ptr_tab[0] = (uint64_t)&page_dir | 1; // set the page directory into the PDPT and mark it present
//    page_dir[0] = (uint64_t)&p_tab | 3; //set the page table into the PD and mark it present/writable
//
//    unsigned int i, address = 0;
//    for(i = 0; i < 512; i++)
//    {
//        page_tab[i] = address | 3; // map address and mark it present/writable
//        address = address + 0x1000;
//    }
//
//    // load it
//    asm volatile ("movl %cr4, %eax; bts $5, %eax; movl %eax, %cr4"); // set bit5 in CR4 to enable PAE
//    asm volatile ("movl %%eax, %%cr3" :: "a" (&page_dir_ptr_tab)); // load PDPT into CR3
//
//    // activate
//    asm volatile ("movl %cr0, %eax; orl $0x80000000, %eax; movl %eax, %cr0;");
//
//    // map the directory to itself
//    uint64_t * page_dir = (uint64_t*)page_dir_ptr_tab[3]; // get the page directory (you should 'and' the flags away)
//    page_dir[511] = (uint64_t)page_dir; // map pd to itself
//    page_dir[510] = page_dir_ptr_tab[2]; // map pd3 to it
//    page_dir[509] = page_dir_ptr_tab[1]; // map pd2 to it
//    page_dir[508] = page_dir_ptr_tab[0]; // map pd1 to it
//    page_dir[507] = (uint64_t)&page_dir_ptr_tab; // map the PDPT to the directory
//                                                  
//    // NOTE: http://wiki.osdev.org/Setting_Up_Paging_With_PAE
//    // Now you can access all structures in virtual memory. Mapping the PDPT into the directory wastes quite much virtual memory as only 32 bytes are used, but if you allocate most/all PDPT's into one page then you can access ALL of them, which can be quite useful You can also statically allocate the PDPT at boot time, put the 4 page directory addresses in your process struct, and then just write the same PDPT address to CR3 on a context switch after you've patched the PDPT.
//}




//////////////////////////////////////////////////
// Paging

//u32 page_directory[1024] __attribute__((aligned(4096)));
//u32 first_page_table[1024] __attribute__((aligned(4096)));
// TODO: if you want more than 4K dir * 4K pages need to add another level


//
// US RW  P - Description
// 0  0  0 - Supervisory process tried to read a non-present page entry
// 0  0  1 - Supervisory process tried to read a page and caused a protection fault
// 0  1  0 - Supervisory process tried to write to a non-present page entry
// 0  1  1 - Supervisory process tried to write a page and caused a protection fault
// 1  0  0 - User process tried to read a non-present page entry
// 1  0  1 - User process tried to read a page and caused a protection fault
// 1  1  0 - User process tried to write to a non-present page entry
// 1  1  1 - User process tried to write a page and caused a protection fault
void page_fault_handler(isr_stack_state* r)
{
    u32 i = r->int_no;
    u32 err = r->err_code;
    trace("page fault handler (isr #%d) called with err: %x", i, err);

    //k_panic();
}

// TODO:
/*
 Creating a Blank Page Directory

 The first step is to create a blank page directory. The page directory is blank because we have not yet
 created any page tables where the entries in the page directory can point.

 Note that all of your paging structures need to be at page-aligned addresses (i.e. being a multiple of 4096).
 If you have already written a page frame allocator then you can use it to allocate the first free page after
 your kernel for the page directory. If you have not created a proper page allocator, simply finding the first
 free page-aligned address after the kernel will be fine, but you should write the page frame allocator as
 soon as possible. Another temporary solution (used in this tutorial) is to simply declare global objects 
 with __attribute__((align(4096))). Note that this is a GCC extension. It allows you to declare data aligned 
 with some mark, such as 4KiB here. We can use this because we are only using one page directory and one page 
 table. Please note that on the real world, dynamic allocation is too basic to be missing, and paging 
 structures are constantly being added, deleted, and modified. For now, just use static objects;
 */
void init_page_directory() 
{
    trace("init multitasking\n");

    // The size of physical memory. For the moment we
    // assume it is 16MB big.
    u32 mem_end_page = 0x1000000;

    nframes = mem_end_page / 0x1000;
    frames = (u32*)kmalloc(INDEX_FROM_BIT(nframes));
    kmemset(frames, 0, INDEX_FROM_BIT(nframes));

    trace("nframes = %d\n", nframes);

    // Let's make a page directory.
    //TODO: u32 phys;
    kernel_directory = (page_directory*)kmalloc_a(sizeof(page_directory));
    kmemset(kernel_directory, 0, sizeof(page_directory));

    trace("kernel_dir: tables = %p, tablesPhys = %p\n", kernel_directory->tables, kernel_directory->tablesPhysical);
    kernel_directory->physicalAddress = (u32)kernel_directory->tablesPhysical;

    u32 i = 0;
    for (i = sys_heap_bottom; i < sys_heap_top; i += 0x1000)
        get_page(i, 1, kernel_directory);

    i = 0;
    while (i < free_ptr + 0x1000)
    {
        // Kernel code is readable but not writeable from userspace.
        alloc_frame( get_page(i, 1, kernel_directory), 0, 0);
        i += 0x1000;
    }

    // Now allocate those pages we mapped earlier.
    for (i = sys_heap_bottom; i < sys_heap_bottom + KHEAP_INITIAL_SIZE; i += 0x1000)
        alloc_frame( get_page(i, 1, kernel_directory), 0, 0);

    trace("loading in page directory\n");

    // must have page fault handler installed before enabling paging otherwise GPF calling `trace()` or other
    const int ISR_PAGE_FAULT = 0x0e;
    isr_install_handler(ISR_PAGE_FAULT, page_fault_handler, "page fault");

    trace("switghing to kernel page directory\n");

    // Now, enable paging!
    switch_page_directory(kernel_directory);

    trace("creating kernel heap\n");

    // Initialise the kernel heap.
    heap_ptr = (u8*)create_heap(KHEAP_START, KHEAP_START + KHEAP_INITIAL_SIZE, 0xCFFFF000, 0, 0);

//    current_directory = clone_directory(kernel_directory);
//    switch_page_directory(current_directory);

    trace("paging enabled\n");
}

void switch_page_directory(page_directory* dir)
{
    current_directory = dir;

    trace("switching to page: phys = %x, \n", dir->physicalAddress);

    // load page directory
    asm volatile("mov %0, %%cr3" :: "r"(dir->physicalAddress));

    // enable paging (bit 32)
    u32 cr0;
    asm volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0":: "r"(cr0));
}

// TODO: I believe this is for 64-bit
void enable_pse()
{
    u32 cr4;
    asm volatile("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= 0x00000010; //
    asm volatile("mov %0, %%cr4" :: "r"(cr4));
}

//void invalidate_page_tables() {}
//void invalidate_page(page_table_entry* page)
//{
//    asm volatile("invlpg %0" :: "=m"(page->frameAddress));
//}

page_table_entry* get_page(u32 address, int make, page_directory *dir)
{
    // Turn the address into an index.
    address /= 0x1000;

    // Find the page table containing this address.
    u32 table_idx = address / 1024;

    if (dir->tables[table_idx]) // If this table is already assigned
    {
        return &dir->tables[table_idx]->pages[address%1024];
    }
    else if(make)
    {
        u32 tmp;
        dir->tables[table_idx] = (page_table*)kmalloc_ap(sizeof(page_table), &tmp);
        kmemset(dir->tables[table_idx], 0, 0x1000);
        dir->tablesPhysical[table_idx] = tmp | 0x7; // PRESENT, RW, US.
        return &dir->tables[table_idx]->pages[address%1024];
    }
    else
    {
        return 0;
    }
}

void map_page(void* physaddr, void* virtualaddr, i32 flags)
{
    // Make sure that both addresses are page-aligned.

    u32 pdindex = (u32)virtualaddr >> 22;
    u32 ptindex = (u32)virtualaddr >> 12 & 0x03FF;

    //u32 * pd = (u32 *)0xFFFFF000;

    // Here you need to check whether the PD entry is present.

    // When it is not present, you need to create a new empty PT and
    // adjust the PDE accordingly.


    u32 * pt = ((u32 *)0xFFC00000) + (0x400 * pdindex);
    // Here you need to check whether the PT entry is present.
    // When it is, then there is already a mapping present. What do you do now?

    pt[ptindex] = ((u32)physaddr) | (flags & 0xFFF) | 0x01; // Present

    // Now you need to flush the entry in the TLB
    // or you might not notice the change.
}

static page_table* clone_table(page_table* src, u32* physAddr)
{
    // Make a new page table, which is page aligned.
    page_table* table = (page_table*)kmalloc_ap(sizeof(page_table), physAddr);

    // Ensure that the new table is blank.
    kmemset(table, 0, sizeof(page_directory));

    // For every entry in the table...
    int i;
    for (i = 0; i < 1024; i++)
    {
        if (!src->pages[i].frameAddress)
            continue;

        // Get a new frame.
        alloc_frame(&table->pages[i], 0, 0);

        // Clone the flags from source to destination.
        if (src->pages[i].present)      table->pages[i].present = 1;
        if (src->pages[i].readwrite)    table->pages[i].readwrite = 1;
        if (src->pages[i].accessRing3)  table->pages[i].accessRing3 = 1;
        if (src->pages[i].accessed)     table->pages[i].accessed = 1;
        if (src->pages[i].dirty)        table->pages[i].dirty = 1;

        // Physically copy the data across. This function is in process.s.
        copy_page_physical(src->pages[i].frameAddress * 0x1000, table->pages[i].frameAddress * 0x1000);
        
    }
    return table;
}

page_directory* clone_directory(page_directory* src)
{
    u32 phys;

    // Make a new page directory and obtain its physical address.
    page_directory* dir = (page_directory*)kmalloc_ap(sizeof(page_directory), &phys);

    // Ensure that it is blank.
    kmemset(dir, 0, sizeof(page_directory));

    // Get the offset of tablesPhysical from the start of the page_directory_t structure.
    u32 offset = (u32)dir->tablesPhysical - (u32)dir;

    // Then the physical address of dir->tablesPhysical is:
    dir->physicalAddress = phys + offset;

    int i;
    for (i = 0; i < 1024; i++)
    {
        if (! src->tables[i])
            continue;

        if (kernel_directory->tables[i] == src->tables[i])
        {
            // It's in the kernel, so just use the same pointer.
            dir->tables[i] = src->tables[i];
            dir->tablesPhysical[i] = src->tablesPhysical[i];
        }
        else
        {
            // Copy the table.
            u32 phys;
            dir->tables[i] = clone_table(src->tables[i], &phys);
            dir->tablesPhysical[i] = phys | 0x07;
        }
    }
    return dir;
}



//// Abstract model of a TLB.
//
//
//// Flag to mark an entry in the modelled hardware TLB as having been set for use as a valid translation.
//#define TLB_ENTRY_FLAGS_INUSE
//#define CPU_MODEL_MAX_TLB_ENTRIES 10
//
//typedef struct
//{
//    vaddr_t entry_virtual_address;
//    paddr_t relevant_physical_address;
//    uint16_t permissions;
//} tlb_cache_record;
//
//// Instance of a hardware Translation Lookaside Buffer.
//
//internal tlb_cache_record hw_tlb[CPU_MODEL_MAX_TLB_ENTRIES];
//
//// Model routine for a TLB lookup.
//
//int tlb_lookup(vaddr_t v, paddr_t *p)
//{
//    for (int i=0; i<CPU_MODEL_MAX_TLB_ENTRIES; i++)
//    {
//        if (hw_tlb[i].permissions & TLB_ENTRY_FLAGS_INUSE && hw_tlb[i].entry_virtual_address == v)
//        {
//            *p = hw_tlb[i].relevant_physical_address;
//            return 1;
//        };
//    };
//    return 0;
//}
//
//void tlb_flush()
//{
//    asm volatile ("tlbflsh %0\n\t" : : "r" (virtual_address));
//}
//
//// Modelled function for a flush of the TLB modelled earlier on.
//
//void tlb_flush_single(vaddr_t v)
//{
//    for (int i=0; i<CPU_MODEL_MAX_TLB_ENTRIES; i++)
//    {
//        if (hw_tlb[i].permissions & TLB_ENTRY_FLAGS_INUSE && hw_tlb[i].entry_virtual_address == v)
//        {
//            ht_tlb[i].permissions &= ~TLB_ENTRY_FLAGS_INUSE;
//            return;
//        };
//    };
//}