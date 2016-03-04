;**********************************
; Author: Steve Tranby
; Using NASM, GCC (for now), ELF
; TranbyOS Boot and Assembly Hooks
; GRUB MultiBoot Loader 
;**********************************

; Utilize 32-bit x86
[BITS 32]

global start
global gdt_flush	       ; Allows the C code to link to this
global tss_flush	       ; Allows the C code to link to this
global idt_load		       ; Allows the C code to link to this
global _sys_heap	       ; Allows the C code to link to this 
global _jump_usermode      ; 

extern _kmain		       ; have to specify '_main' instead of 'main' since we're using ELF format
extern gp                  ; Says that 'gp' is in another file
extern idtp                ;
extern fault_handler       ;
extern irq_handler         ;
extern _test_user_function ;

start:
    mov esp, _sys_stack		; This points the stack to our new stack area
    jmp stublet

; This part MUST be 4byte aligned, so we solve that issue using 'ALIGN 4'
align 4
mboot:
    ; Multiboot macros to make a few lines later more readable
    MULTIBOOT_HEADER_MAGIC	equ 0x1BADB002

    MULTIBOOT_PAGE_ALIGN	equ 1<<0
    MULTIBOOT_MEMORY_INFO	equ 1<<1
    MULTIBOOT_VIDEO_INFO    equ 1<<2
    MULTIBOOT_AOUT_KLUDGE	equ 1<<16
    MULTIBOOT_HEADER_FLAGS	equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO | MULTIBOOT_VIDEO_INFO | MULTIBOOT_AOUT_KLUDGE
;    MULTIBOOT_HEADER_FLAGS	equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO | MULTIBOOT_VIDEO_INFO
    MULTIBOOT_CHECKSUM		equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

    EXTERN code, bss, end

    ; This is the GRUB Multiboot header. A boot signature
    dd MULTIBOOT_HEADER_MAGIC
    dd MULTIBOOT_HEADER_FLAGS
    dd MULTIBOOT_CHECKSUM
    
    ; AOUT kludge - must be physical addresses. Make a note of these:
    ; The linker script fills in the data for these ones!
    dd mboot    ; header
    dd code     ; load addr
    dd bss      ; load end, bss start
    dd end      ; bss end
    dd start    ; entry

    ; Request linear graphics mode
    dd 0        ; request mode type
    dd 0        ; request width
    dd 0        ; request height
    dd 32

; call our main() function 
stublet:
	push eax    ; contains 0x2BADB002
	push ebx    ; header physical address
    cli
	call _kmain
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

; Load the index of our TSS structure - The index is
; 0x28, as it is the 5th selector and each is 8 bytes
; long, but we set the bottom two bits (making 0x2B)
; so that it has an RPL of 3, not zero.
; Load 0x2B into the task state register.
tss_flush:
    mov ax, 0x2B
    ltr ax
    ret

; Will switch over to Ring 3 
_jump_usermode:
     mov ax,0x23
     mov ds,ax
     mov es,ax 
     mov fs,ax 
     mov gs,ax      ;we don't need to worry about SS. it's handled by iret
 
     mov eax,esp
     push 0x23      ;user data segment with bottom 2 bits set for ring 3
     push eax       ;push our current stack just for the heck of it
     pushf
     push 0x1B      ;user code segment with bottom 2 bits set for ring 3
     push _test_user_function ;may need to remove the _ for this to work right 
     iret

;--------------------------------------------------------------------

; Loads the IDT defined in '_idtp' into the processor.
; This is declared in C as 'extern void idt_load();'
idt_load:
    lidt [idtp]
    ret

;--------------------------------------------------------------------
; Interrupt Service Routines (ISR)

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

    iret           ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP!

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
SECTION .bss
    resb 16384               ; This reserves 8KBytes of memory here for the stack
_sys_stack:
	resb 16384              ; This is the heap
_sys_heap:
