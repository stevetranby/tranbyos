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
u8* free_ptr;
//static block_t *freelist;

// Initialize the Memory Manager
void init_mm()
{
    kmemset(blocks_used, 0, MAX_BLOCKS);
    // Let's start simple and create just a standard heap
    heap_ptr = (u8*)&sys_heap_bottom + 1;
    free_ptr = heap_ptr;

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
    trace( "print_heap_bytes:\n\n");
    int i = 0;
    for(u32* ptr = (u32*)heap_ptr; i < n; ++i, ++ptr) {
        kserialf( "%x", *ptr);
        if(0 == (i+1) % 8) kserialf( "\n");
        else kserialf( " ");
    }
    kserialf("\n");
}

// TODO: a lot of potential places to macro or pre-proc out the excess
// TODO: do we want this? it's a nice handy shortcut
#define FOR(n) for(int i=0; i<10;)

void print_blocks_avail()
{
    trace("blocks available:\n\n");
    for(int i = 0; i < MAX_BLOCKS; ++i)
    {
        kserialf( "%d", blocks_used[i]);
        if(0 == (i+1) % 60) kserialf("\n");
        else kserialf( ",");
    }
    kserialf("\n");
}

// TODO: allocate extra block on each side and fill with debug markers
// TODO: make block size larger (multiple sizes)
// Byte Allocator for Heap
u8* kmalloc_b(u32 nblks)
{
    u32 offset = free_ptr - heap_ptr;

    // TODO: wrap around search and try to find contiguous space in freed blocks
    //       if wrapping need to check blocks_used[] to make sure it's free
    if((u32)offset + nblks > MAX_BLOCKS) {
        trace("TODO: need to have wrap around malloc\n");
		return NULL;
    }

    // mark used
    trace( "blocks_used %p, %d, %d, %d\n", blocks_used + offset, offset, nblks, nblks);
    kmemset(blocks_used + offset, nblks, nblks);

    // return init block (debug fill with known data)
    u8* tmp = free_ptr;
	free_ptr = free_ptr + nblks;
#ifdef _DEBUG_
    kmemset(tmp, 0xee, nblks);
#endif
    trace( "malloc ret ptr = %p\n", tmp);
	return tmp;
}

//void* malloc(u32 size)
//{
//	// Find free block(s) for memory of size nbytes	
//	// Set that block(s) to used
//	// Possibly store some extra information
//	// Return the address of this block
//	return NULL;
//}

void kfree_b(u8* addr)
 {
     u32 start = addr - heap_ptr;
     u32 nblks = blocks_used[start];
     kmemset(blocks_used, 0, nblks);

	// Find block(s) starting at memory address of addr
	// Set these block(s) to free!
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

u32 pageDirectory[1024] __attribute__((aligned(4096)));
u32 firstPageTable[1024] __attribute__((aligned(4096)));
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

    //set each entry to not present
    for(int i = 0; i < 1024; i++)
    {
        // This sets the following flags to the pages:
        // bit 0 = 0 - not present
        // bit 1 = 1 - writable
        // bit 2 = 0 - supervisor only
        pageDirectory[i] = PAGE_DIR_READWRITE;
    }

    //we will fill all 1024 entries in the table, mapping 4 megabytes
    for(int i = 0; i < 1024; i++)
    {
        // As the address is page aligned, it will always leave 12 bits zeroed.
        // attributes: supervisor level (0), read/write (1), present (1)
        firstPageTable[i] = (i * PAGE_DIR_ADDR_BASE) | PAGE_DIR_READWRITE | PAGE_DIR_PRESENT;
    }

    // attributes: supervisor level (0), read/write (1), present (1)
    //page_directory[0] = ((unsigned int)first_page_table) | 3;
    pageDirectory[0] = (u32)firstPageTable | PAGE_DIR_READWRITE | PAGE_DIR_PRESENT;

    trace("loading in page directory\n");

    // must have page fault handler installed before enabling paging otherwise GPF calling `trace()` or other
    const int ISR_PAGE_FAULT = 0x0e;
    isr_install_handler(ISR_PAGE_FAULT, page_fault_handler, "page fault");
    loadPageDirectory(pageDirectory);
    enablePaging();

    trace("paging enabled\n");
}


// from - virtual address to start Ident paging
// size - # bytes to page from start(from)
void idpaging(uint32_t *first_pte, vaddr from, int size);
void idpaging(uint32_t *first_pte, vaddr from, int size)
{
    // discard bits we don't want
    from = from & 0xfffff000;
    for(; size > 0; from += 4096, size -= 4096, ++first_pte)
    {
        *first_pte = from|1;     // mark page present.
    }
}

void* get_physaddr(void* virtualaddr)
{
    u32 pdindex = (u32)virtualaddr >> 22;
    u32 ptindex = (u32)virtualaddr >> 12 & 0x03FF;

    //u32 * pd = (u32*)0xFFFFF000;

    u32 pd = pageDirectory[pdindex];
    UNUSED_VAR(pd);

    // Here you need to check whether the PD entry is present.
    // TODO: if(BIT(pd, PAGE_DIR_PRESENT))

    u32 pt = 0xFFC00000 + (0x400 * pdindex);
    UNUSED_VAR(pt);

    // Here you need to check whether the PT entry is present.

    u32 base = firstPageTable[ptindex] & ~0xFFF;
    u32 offset = (u32)virtualaddr & 0xFFF;
    return (void *)(base + offset);
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