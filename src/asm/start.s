;**********************************
; Author: Steve Tranby
; Using NASM, GCC (for now), ELF
; TranbyOS Boot and Assembly Hooks
; GRUB MultiBoot Loader 
;**********************************

; Utilize 32-bit x86
[BITS 32]

; This is the kernel's entry point. We could either call main here,
; or we can use this to setup the stack or other nice stuff, like
; perhaps setting up the GDT and segments. Please note that interrupts
; are disabled at this point: More on interrupts later!
[BITS 32]
global start
global gdt_flush	; Allows the C code to link to this
global idt_load		; Allows the C code to link to this
global _sys_heap	; Allows the C code to link to this 

extern _main		; have to specify '_main' instead of 'main' since we're using ELF format
extern gp		; Says that 'gp' is in another file
extern idtp

start:
    mov esp, _sys_stack		; This points the stack to our new stack area
    jmp stublet

; This part MUST be 4byte aligned, so we solve that issue using 'ALIGN 4'
align 4
mboot:
    ; Multiboot macros to make a few lines later more readable
    MULTIBOOT_PAGE_ALIGN	equ 1<<0
    MULTIBOOT_MEMORY_INFO	equ 1<<1
    MULTIBOOT_VIDEO_INFO   equ 1<<2
    MULTIBOOT_AOUT_KLUDGE	equ 1<<16
    MULTIBOOT_HEADER_MAGIC	equ 0x1BADB002
    MULTIBOOT_HEADER_FLAGS	equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO | MULTIBOOT_VIDEO_INFO | MULTIBOOT_AOUT_KLUDGE
    MULTIBOOT_CHECKSUM		equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
    EXTERN code, bss, end

    ; This is the GRUB Multiboot header. A boot signature
    dd MULTIBOOT_HEADER_MAGIC
    dd MULTIBOOT_HEADER_FLAGS
    dd MULTIBOOT_CHECKSUM
    
    ; AOUT kludge - must be physical addresses. Make a note of these:
    ; The linker script fills in the data for these ones!
    dd mboot
    dd code
    dd bss
    dd end
    dd start


; call our main() function 
stublet:      
    ;call _main 	; elf gcc puts _ in front of function names    	
	push eax
	push ebx
	call _main
    jmp $           ; jump to self (halta)


sse_enabled:
    mov eax, 0x1
    cpuid
    test edx, 1<<25
    jz .noSSE
    ;SSE is available
    ret;

.noSSE:
    ret;

enable_sse:
    ;now enable SSE and the like
    mov eax, cr0
    and ax, 0xFFFB      ;clear coprocessor emulation CR0.EM
    or ax, 0x2          ;set coprocessor monitoring  CR0.MP
    mov cr0, eax
    mov eax, cr4
    or ax, 3 << 9       ;set CR4.OSFXSR and CR4.OSXMMEXCPT at the same time
    mov cr4, eax
    ret

_outp_8:
    ;outb 
    ret;
_outp_16:
    ;outw
    ret;
_outp_32:
    ;outl
    ret; 
_inp_8:
    ;inb
    ret;
_inp_16:
    ;inw
    ret;
_inp_32:
    ;inl
    ret;


; Shortly we will add code for loading the GDT right here!
; This will set up our new segment registers. We need to do
; something special in order to set CS. We do what is called a
; far jump. A jump that includes a segment as well as an offset.
; This is declared in C as 'extern void gdt_flush();'
gdt_flush:
    lgdt [gp]        ; Load the GDT with our '_gp' which is a special pointer
    mov ax, 0x10      ; 0x10 is the offset in the GDT to our data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:flush2   ; 0x08 is the offset to our code segment: Far jump!
flush2:
    ret               ; Returns back to the C code!


; Loads the IDT defined in '_idtp' into the processor.
; This is declared in C as 'extern void idt_load();'
idt_load:
    lidt [idtp]
    ret

; Interrupt Service Routines (ISR)
%assign i 0
%rep    32

; make handler available to 'C'
global isr%[i]

; IRQ handler
isr%[i]:
    cli
    push byte 0
    push byte %[i]
    jmp isr_common_stub

%assign i i+1
%endrep
	
; We call a C function in here. We need to let the assembler know
; that '_fault_handler' exists in another file
extern fault_handler

; This is our common ISR stub. It saves the processor state, sets
; up for kernel mode segments, calls the C-level fault handler,
; and finally restores the stack frame.
isr_common_stub:
    pusha
    push ds
    push es
    push fs
    push gs
    mov ax, 0x10   ; Load the Kernel Data Segment descriptor!
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, esp   ; Push us the stack
    push eax
    mov eax, fault_handler
    call eax       ; A special call, preserves the 'eip' register
    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8     ; Cleans up the pushed error code and pushed ISR number
    iret           ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP!

; Interrupt Request (IRQ) handlers
%assign i 0
%assign j 32
%rep    16

; make handler available to 'C'
global irq%[i]

; IRQ handler
irq%[i]:
    cli
    push byte 0
    push byte %[j]
    jmp irq_common_stub

%assign i i+1
%assign j j+1
%endrep

extern irq_handler

irq_common_stub:
    pusha
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, esp

    push eax
    mov eax, irq_handler
    call eax
    pop eax

    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    iret

; Here is the definition of our BSS section. Right now, we'll use
; it just to store the stack. Remember that a stack actually grows
; downwards, so we declare the size of the data before declaring
; the identifier '_sys_stack'
SECTION .bss
    resb 8192               ; This reserves 8KBytes of memory here for the stack
_sys_stack:
	resb 1024
_sys_heap:
