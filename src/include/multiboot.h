#pragma once

// The magic number for the Multiboot header.
#define MULTIBOOT_HEADER_MAGIC          0x1BADB002
// The flags for the Multiboot header. (ELF or AOUT Kluge)
#ifdef __ELF__
# define MULTIBOOT_HEADER_FLAGS         0x00000003
#else
# define MULTIBOOT_HEADER_FLAGS         0x00010003
#endif
// The magic number passed by a Multiboot-compliant boot loader. */
#define MULTIBOOT_BOOTLOADER_MAGIC      0x2BADB002
// The size of our stack (16KB)
#define STACK_SIZE                      0x4000
/* C symbol format. HAVE_ASM_USCORE is defined by configure. */
#ifdef HAVE_ASM_USCORE
# define EXT_C(sym)                     _ ## sym
#else
# define EXT_C(sym)                     sym
#endif

// The Multiboot header.
typedef struct
{
	u32 magic;
	u32 flags;
	u32 checksum;
	u32 header_addr;
	u32 load_addr;
	u32 load_end_addr;
	u32 bss_end_addr;
	u32 entry_addr;
} multiboot_header_t;

// The symbol table for a.out.
typedef struct
{
	u32 tabsize;
	u32 strsize;
	u32 addr;
	u32 reserved;
} aout_symbol_table_t;

// The section header table for ELF.
typedef struct
{
	u32 num;
	u32 size;
	u32 addr;
	u32 shndx;
} elf_section_header_table_t;

// The Multiboot information.
// See: https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#Boot_002dtime-configuration
typedef struct
{
	u32 flags;
	
	u32 mem_lower;
	u32 mem_upper;
	
	u32 boot_device;
	u32 cmdline;
	
	u32 mods_count;
	u32 mods_addr;
	
	union
	{
		aout_symbol_table_t aout_sym;
		elf_section_header_table_t elf_sec;
	} u;
	
	u32 mmap_length;
	u32 mmap_addr;

	u32 drives_length;
	u32 drives_addr;
	
	u32 config_table;
	
	u32 boot_loader_name;
	
	u32 apm_table;

	// TODO: create vbe struct 
	u32 vbe_control_info;
	u32 vbe_mode_info;
	u32 vbe_mode;
	u32 vbe_interface_seg;
	u32 vbe_interface_off;
	u32 vbe_interface_len;

} multiboot_info_t;

/* The module structure. */
typedef struct
{
	unsigned long mod_start;
	unsigned long mod_end;
	unsigned long string;
	unsigned long reserved;
} module_t;

/* The memory map. Be careful that the offset 0 is base_addr_low
but no size. */
typedef struct
{
	unsigned long size;
	unsigned long base_addr_low;
	unsigned long base_addr_high;
	unsigned long length_low;
	unsigned long length_high;
	unsigned long type;
} memory_map_t;
