#include <system.h>

//static inline u8 inb_dummy(u16 port)
//{
//    u8 ret;
//    asm volatile ( "inb %[port], %[ret]"
//                  : [ret] "=a"(ret)
//                  : [port] "Nd"(port) );
//    return ret;
//}

// TODO: maybe just move these into .asm? why bother just because
//       we want everything in C?


/// Read from IO port (8,16,32-bit value)
u8 inb (u16 _port)
{
    u8 rv;
    asm volatile ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

u16 inw(u16 _port)
{
    u16 rv;
    asm volatile ("inw %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

u32 inl(u16 _port)
{
    u32 rv;
    asm volatile ("inl %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

/// Write to IO port (8,16,32-bit value)
void outb(u16 _port, u8 _data)
{
    asm volatile ("outb %1, %0" : : "dN" (_port), "a" (_data));
}

void outw(u16 _port, u16 _data)
{
    asm volatile ("outw %1, %0" : : "dN" (_port), "a" (_data));
}

void outd(u16 _port, u32 _data)
{
    asm volatile ("outl %1, %0" : : "dN" (_port), "a" (_data));
}

///*
// * memsetw
// * Set `count` shorts to `val`.
// */
//unsigned short * memsetw(unsigned short * dest, unsigned short val, int count) {
//    int i = 0;
//    for ( ; i < count; ++i ) {
//        dest[i] = val;
//    }
//    return dest;
//}
//
//uint32_t __attribute__ ((pure)) krand(void) {
//    static uint32_t x = 123456789;
//    static uint32_t y = 362436069;
//    static uint32_t z = 521288629;
//    static uint32_t w = 88675123;
//
//    uint32_t t;
//
//    t = x ^ (x << 11);
//    x = y; y = z; z = w;
//    return w = w ^ (w >> 19) ^ t ^ (t >> 8);
//}


//void outportsm(unsigned short port, unsigned char * data, u32 size) {
//    asm volatile ("rep outsw" : "+S" (data), "+c" (size) : "d" (port));
//}
//
//void inportsm(unsigned short port, unsigned char * data, u32 size) {
//    asm volatile ("rep insw" : "+D" (data), "+c" (size) : "d" (port) : "memory");
//}
//
//size_t lfind(const char * str, const char accept) {
//    return (size_t)strchr(str, accept);
//}
//
//size_t rfind(const char * str, const char accept) {
//    return (size_t)strrchr(str, accept);
//}
//
//uint8_t startswith(const char * str, const char * accept) {
//    return strstr(str, accept) == str;
//}

//static inline u32 farpeekl(u16 sel, void* off)
//{
//    u32 ret;
//    asm volatile ( "push %%fs\n\t"
//                     "mov  %1, %%fs\n\t"
//                     "mov  %%fs:(%2), %0\n\t"
//                     "pop  %%fs"
//                     : "=r"(ret) : "g"(sel), "r"(off) );
//    return ret;
//}
//
//static inline void farpokeb(u16 sel, void* off, u8 v)
//{
//    asm volatile ( "push %%fs\n\t"
//                     "mov  %0, %%fs\n\t"
//                     "movb %2, %%fs:(%1)\n\t"
//                     "pop %%fs"
//                     : : "g"(sel), "r"(off), "r"(v) );
//    // TODO: Should "memory" be in the clobber list here?
//}
//
//// GCC has a <cpuid.h> header and intel has intrinsics header
//bool cpuid_exists_by_eflags(void)
//{
//    // TODO: check if exists on x86 (always present on x86_64)
//    // return 1;
//
//    i32 result;
//    asm volatile (  "	pushfl\n"
//                    "	pop	%%eax\n"
//                    "	mov	%%eax,	%%ecx\n"
//                    "	xor	$0x200000,	%%eax\n"
//                    "	push	%%eax\n"
//                    "	popfl\n"
//                    "	pushfl\n"
//                    "	pop	%%eax\n"
//                    "	xor	%%ecx,	%%eax\n"
//                    "	mov	%%eax,	%0\n"
//                    "	push	%%ecx\n"
//                    "	popfl\n"
//                    : "=m"(result) : : "eax", "ecx", "memory");
//    return (result != 0);
//}

//static inline void cpuid(int code, u32* a, u32* d)
//{
//   asm volatile ( "cpuid" : "=a"(*a), "=d"(*d) : "0"(code) : "ebx", "ecx" );
//}

///// CPU's time-stamp counter and store into EDX:EAX.
//static inline u64 rdtsc()
//{
//    u64 ret;
//    asm volatile ( "rdtsc" : "=A"(ret) );
//    return ret;
//}
//
//static inline bool are_interrupts_enabled()
//{
//    u32 flags;
//    asm volatile ( "pushf\n\t"
//                  "pop %0"
//                  : "=g"(flags) );
//    return flags & (1 << 9);
//}
//
//static inline void lidt(void* base, u16 size)
//{   // This function works in 32 and 64bit mode
//    struct PACKED {
//        u16 length;
//        void*    base;
//    } IDTR = { size, base };
//
//    asm volatile ( "lidt %0" : : "m"(IDTR) );  // let the compiler choose an addressing mode
//}
//
//static inline u32 read_cr0(void)
//{
//    u32 val;
//    asm volatile ( "mov %%cr0, %0" : "=r"(val) );
//    return val;
//}

// Invalidates the TLB (Translation Lookaside Buffer) for one specific virtual address. The next memory reference for the page will be forced to re-read PDE and PTE from main memory. Must be issued every time you update one of those tables. The m pointer points to a logical address, not a physical or virtual one: an offset for your ds segment.
inline void invlpg(void* m)
{
    /* Clobber memory to avoid optimizer re-ordering access before invlpg, which may cause nasty bugs. */
    asm volatile ( "invlpg (%0)" : : "b"(m) : "memory" );
}

// Write a 64-bit value to a MSR. The A constraint stands for concatenation of registers EAX and EDX.
inline void wrmsr(u32 msr_id, u64 msr_value)
{
    asm volatile ( "wrmsr" : : "c" (msr_id), "A" (msr_value) );
}

// Read a 64-bit value from a MSR. The A constraint stands for concatenation of registers EAX and EDX.
inline u64 rdmsr(u32 msr_id)
{
    u64 msr_value;
    asm volatile ( "rdmsr" : "=A" (msr_value) : "c" (msr_id) );
    return msr_value;
}

