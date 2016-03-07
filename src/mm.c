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
extern u8* sys_heap;
extern u8* prog_bss_end;

//static void *brkval;
u8 *heap_ptr;
u8 *free_ptr;
//static block_t *freelist;

// Initialize the Memory Manager
void init_mm()
{
	// Let's start simple and create just a standard heap
	heap_ptr = sys_heap + 1;
	heap_ptr[0] = 'H';
	heap_ptr[1] = 'E';
	heap_ptr[2] = 'A';
	heap_ptr[3] = 'P';
    heap_ptr[4] = '\0';
	free_ptr = heap_ptr + 5;
}

void print_heap_magic()
{
    trace("heap magic\n");
    kprintf("Heap: %x | %x | %x\n", (u32)sys_heap, (u32)heap_ptr, (u32)prog_bss_end);
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
//
////////////////////////////////////////////////////
//// Paging
//
//// And this inside a function
//i32 page_directory[1024] __attribute__((aligned(4096)));
//i32 first_page_table[1024] __attribute__((aligned(4096)));
//
//void init_page_directory() 
//{
//	void loadPageDirectory(i32* page_directory);
//	void enablePaging();
//
//	//set each entry to not present
//	for(i32 i = 0; i < 1024; i++)
//	{
//	    // This sets the following flags to the pages:
//	    //   Supervisor: Only kernel-mode can access them
//	    //   Write Enabled: It can be both read from and written to
//	    //   Not Present: The page table is not present
//	    page_directory[i] = 0x00000002;
//	}
//
//	// holds the physical address where we want to start mapping these pages to.
//	// in this case, we want to map these pages to the very beginning of memory.
//
//	//we will fill all 1024 entries in the table, mapping 4 megabytes
//	for(i32 i = 0; i < 1024; i++)
//	{
//	    // As the address is page aligned, it will always leave 12 bits zeroed.
//	    // Those bits are used by the attributes ;)
//	    first_page_table[i] = (i * 0x1000) | 3; // attributes: supervisor level, read/write, present.
//	}
//
//	// attributes: supervisor level, read/write, present
//	page_directory[0] = ((i32)first_page_table) | 3;
//}
//
//void* get_physaddr(void* virtualaddr)
//{
//    uintptr_t pdindex = (uintptr_t)virtualaddr >> 22;
// 	uintptr_t ptindex = (uintptr_t)virtualaddr >> 12 & 0x03FF;
// 
//    void* pd = (void*)0xFFFFF000;
//
//    // Here you need to check whether the PD entry is present.
// 
//    void* pt = ((void*)0xFFC00000) + (0x400 * pdindex);
//    // Here you need to check whether the PT entry is present.
// 
//    return (void*)( (pt[ptindex] & ~0xFFF) + ( (void*)virtualaddr & 0xFFF ) );
//}
//
//void map_page(void* physaddr, void* virtualaddr, i32 flags)
//{
//    // Make sure that both addresses are page-aligned.
// 
//    uintptr_t pdindex = (u64)virtualaddr >> 22;
//    uintptr_t ptindex = (intptr_t)virtualaddr >> 12 & 0x03FF;
// 
//    void* pd = (void*)0xFFFFF000;
//
//    // Here you need to check whether the PD entry is present.
//    // When it is not present, you need to create a new empty PT and
//    // adjust the PDE accordingly.
// 
//    void* pt = ((uintptr_t)0xFFC00000) + (0x400 * pdindex);
//    // Here you need to check whether the PT entry is present.
//    // When it is, then there is already a mapping present. What do you do now?
// 
//    pt[ptindex] = ((uintptr_t)physaddr) | (flags & 0xFFF) | 0x01; // Present
// 
//    // Now you need to flush the entry in the TLB
//    // or you might not notice the change.
//}
//
//
//
//
//// Abstract model of a TLB.
//
//typedef uintptr_t vaddr_t;
//typedef uintptr_t paddr_t;
//
//// Flag to mark an entry in the modelled hardware TLB as having been set for use as a valid translation.
//#define TLB_ENTRY_FLAGS_INUSE
//
//struct tlb_cache_record_t
//{
//    vaddr_t entry_virtual_address;
//    paddr_t relevant_physical_address;
//    uint16_t permissions;
//};
//
//// Instance of a hardware Translation Lookaside Buffer.
//struct tlb_cache_record_t   hw_tlb[CPU_MODEL_MAX_TLB_ENTRIES];
//
//
//// Model routine for a TLB lookup.
//
//int tlb_lookup(vaddr_t v, paddr_t *p)
//{
//    for (int i=0; i<CPU_MODEL_MAX_TLB_ENTRIES; i++)
//    {
//        if (hw_tlb[i].flags & TLB_ENTRY_FLAGS_INUSE && hw_tlb[i].entry_virtual_address == v)
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
//    asm volatile ("TLBFLSH   %0\n\t"::"r" (virtual_address));
//}
//
//// Modelled function for a flush of the TLB modelled earlier on.
//
//void tlb_flush_single(vaddr_t v)
//{
//    for (int i=0; i<CPU_MODEL_MAX_TLB_ENTRIES; i++)
//    {
//        if (hw_tlb[i].flags & TLB_ENTRY_FLAGS_INUSE && hw_tlb[i].entry_virtual_address == v)
//        {
//            ht_tlb[i].flags &= ~TLB_ENTRY_FLAGS_INUSE;
//            return;
//        };
//    };
//}