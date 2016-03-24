;**********************************
; Author: Steve Tranby
; Using NASM, GCC (for now), ELF
; TranbyOS Boot and Assembly Hooks
; GRUB MultiBoot Loader 
;
; Ref: [TODO: osdev wiki link]
;
; Assembly Info
; https://en.wikibooks.org/wiki/X86_Assembly
; https://en.wikibooks.org/wiki/X86_Disassembly/Calling_Conventions
; https://en.wikibooks.org/wiki/X86_Assembly/NASM_Syntax
;**********************************

; Utilize 32-bit x86
[BITS 32]

extern _text_start
extern _text_end
extern _bss_start
extern _bss_end
extern _data_start
extern _data_end

extern kmain
extern gp                 
extern idtp               
extern fault_handler      
extern irq_handler        
extern test_user_function

global _entry
global gdt_flush	    
global tss_flush	    
global idt_load
global sys_stack_bottom
global sys_stack_top
global sys_heap_bottom
global sys_heap_top
global jump_usermode

section .text

_entry:
    mov esp, sys_stack_top		; This points the stack to our new stack area
    jmp stublet

; This part MUST be 4byte aligned, so we solve that issue using 'ALIGN 4'
section .multiboot
align 4
_mboot:
    ; Multiboot macros to make a few lines later more readable
    MULTIBOOT_HEADER_MAGIC	equ 0x1BADB002

    MULTIBOOT_PAGE_ALIGN	equ 1<<0
    MULTIBOOT_MEMORY_INFO	equ 1<<1
    MULTIBOOT_VIDEO_INFO    equ 1<<2
    MULTIBOOT_AOUT_KLUDGE	equ 1<<16
    MULTIBOOT_HEADER_FLAGS	equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO | MULTIBOOT_VIDEO_INFO | MULTIBOOT_AOUT_KLUDGE
    MULTIBOOT_CHECKSUM		equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

    ; This is the GRUB Multiboot header. A boot signature
    dd MULTIBOOT_HEADER_MAGIC
    dd MULTIBOOT_HEADER_FLAGS
    dd MULTIBOOT_CHECKSUM
    
    ; AOUT kludge - must be physical addresses. Make a note of these:
    ; The linker script fills in the data for these ones!
    dd _mboot            ; header
    dd _text_start      ; load addr
    dd _bss_start       ; load end, bss start
    dd _bss_end         ; bss end
    dd _entry           ; entry

    ; Request linear graphics mode
    dd 0        ; request mode type
    dd 0        ; request width
    dd 0        ; request height
    dd 32

section .text

; call our main() function 
stublet:
	push eax    ; contains 0x2BADB002
	push ebx    ; header physical address
    cli
	call kmain
    cli

halt:
    hlt
    jmp $

sse_enabled:
    mov eax, 0x1
    cpuid
    test edx, 1<<25
    jz .noSSE
    ;SSE is available
    ret;
.noSSE:
    ret;

;now enable SSE and the like
enable_sse:
    mov eax, cr0
    and ax, 0xFFFB      ;clear coprocessor emulation CR0.EM
    or ax, 0x2          ;set coprocessor monitoring  CR0.MP
    mov cr0, eax
    mov eax, cr4
    or ax, 3 << 9       ;set CR4.OSFXSR and CR4.OSXMMEXCPT at the same time
    mov cr4, eax
    ret

%macro prologue 1
    push ebp
	mov ebp, esp
	sub	esp, %1
%endmacro
%macro epilogue 0
    mov	esp, ebp
	pop	ebp
	ret
%endmacro
%macro epilogue 1
    mov esp, ebp
    pop ebp
    ret
%endmacro

_outpb:
    prologue 2
    out dx, al
    epilogue
_outpw:
    prologue 2
    out dx, ax
    epilogue
_outpl:
    prologue 2
    out dx, eax
    epilogue
_inpb:
    prologue 1
    in al, dx
    epilogue 1
_inpw:
    prologue 1
    in ax, dx
    epilogue 1
_inpl:
    prologue 1
    in eax, dx
    epilogue 1

;--------------------------------------------------------------------

; Shortly we will add code for loading the GDT right here!
; This will set up our new segment registers. We need to do
; something special in order to set CS. We do what is called a
; far jump. A jump that includes a segment as well as an offset.
; This is declared in C as 'extern void gdt_flush();'
gdt_flush:
    lgdt [gp]         ; Load the GDT with our '_gp' which is a special pointer
    mov ax, 0x10      ; 0x10 is the offset in the GDT to our data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:flush2   ; 0x08 is the offset to our code segment: Far jump!
flush2:
    ret               ; Returns back to the C code!

;--------------------------------------------------------------------

; Loads the IDT defined in '_idtp' into the processor.
; This is declared in C as 'extern void idt_load();'
idt_load:
    lidt [idtp]
    ret

;--------------------------------------------------------------------
; Interrupt Service Routines (ISR)
;
; http://wiki.osdev.org/Exceptions
;
; make handler available to 'C' with `global`
; only push 0 for NO ERROR gates

%assign i 0
%rep    32

global isr %+ i
; IRQ handler
isr %+ i:
    cli
%if (i != 8 && (i < 10 || i > 14))
    push    byte 0
%endif
    push    byte i
    jmp     isr_common_stub
%assign i i+1
%endrep

; This is our common ISR stub. It saves the processor state, sets
; up for kernel mode segments, calls the C-level fault handler,
; and finally restores the stack frame.
isr_common_stub:
    pusha            ; push all regular registers (ax,bx,cx,dx,etc)

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

    popa           ; pop all regular registers (ax,bx,cx,dx,etc)

    add esp, 8     ; Cleans up the pushed error code and pushed ISR number

    ; iret pops 5 things at once: EIP, CS, EFLAGS, ESP, SS
    iret
    ; WRONG pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP!

;--------------------------------------------------------------------
; Interrupt Request (IRQ) handlers
%assign i 0
%assign j 32
%rep    16

; make handler available to 'C'
global irq %+ i

; IRQ handler
irq %+ i:
    cli                     ; make sure we clear interrupts so we have full control
    push byte 0
    push byte j
    jmp irq_common_stub
%assign i i+1
%assign j j+1
%endrep

irq_common_stub:
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

    mov eax, irq_handler
    call eax
    pop eax

    pop gs
    pop fs
    pop es
    pop ds
    popa

    add esp, 8

    sti                     ; make sure we enable interrupts again

    iret

;--------------------------------------------------------------------

; Block Started by Symbol (BSS)
; - mostly used for uninitialized data (static int i;)
; - https://en.wikipedia.org/wiki/.bss
; - often the heap grows upward and thus sys_heap label/extern may be used for
;
; NOTE: using for boot stack/heap
; TODO: adjust once full kernel loaded
;

section .bss

sys_stack_bottom:
    resb 16384               ; This reserves 8KBytes of memory here for the stack
sys_stack_top:
sys_heap_bottom:
	resb 2048              ; This is the test kernel heap
sys_heap_top:
