;
; Save for later, or testing

[BITS 32]

extern test_user_function

;--------------------------------------------------------------------

; Load the index of our TSS structure - The index is
; 0x28, as it is the 5th selector and each is 8 bytes
; long, but we set the bottom two bits (making 0x2B)
; so that it has an RPL of 3, not zero.
; Load 0x2B into the task state register.

section text

global tss_flush
tss_flush:
    mov ax, 0x2B
    ltr ax
    ret

global jump_usermode
jump_usermode:
     mov ax, 0x23
     mov ds, ax
     mov es, ax
     mov fs, ax
     mov gs, ax      ;we don't need to worry about SS. it's handled by iret
     mov eax, esp
     push 0x23      ;user data segment with bottom 2 bits set for ring 3
     push eax       ;push our current stack just for the heck of it
     pushf          ;push the Eflags register
     push 0x1B      ;user data segment with bottom 2 bits set for ring 3
     push test_user_function
     iret


global read_eip
read_eip:
    pop eax                     ; Get the return address
    jmp eax                     ; Return. Can't use RET because return
                                ; address popped off the stack. 

global copy_page_physical
copy_page_physical:
   push ebx              ; According to __cdecl, we must preserve the contents of EBX.
   pushf                 ; push EFLAGS, so we can pop it and reenable interrupts
                         ; later, if they were enabled anyway.
   cli                   ; Disable interrupts, so we aren't interrupted.
                         ; Load these in BEFORE we disable paging!
   mov ebx, [esp+12]     ; Source address
   mov ecx, [esp+16]     ; Destination address

   mov edx, cr0          ; Get the control register...
   and edx, 0x7fffffff   ; and...
   mov cr0, edx          ; Disable paging.

   mov edx, 1024         ; 1024*4bytes = 4096 bytes to copy

.loop:
   mov eax, [ebx]        ; Get the word at the source address
   mov [ecx], eax        ; Store it at the dest address
   add ebx, 4            ; Source address += sizeof(word)
   add ecx, 4            ; Dest address += sizeof(word)
   dec edx               ; One less word to do
   jnz .loop

   mov edx, cr0          ; Get the control register again
   or  edx, 0x80000000   ; and...
   mov cr0, edx          ; Enable paging.

   popf                  ; Pop EFLAGS back.
   pop ebx               ; Get the original value of EBX back.
   ret



global switchTask
switchTask:
    cli;
                            ; arg2, arg1 (esp -= 8)
                            ; eip (esp -= 4)

    pusha                   ; ax,bx,cx,dx(16),sp,bp,si,di(16) (esp -= 32)
    pushf                   ; eflags (esp -= 4)
    
    mov eax, cr3                                         
    push eax                ; cr3 (esp -= 4)

    ; ESP  +  0 ,   4   ,  8 , 12 , 16 , 20 , 24 , 28 , 32 , 36 , 40 , 44 , 48
    ; Stack: cr3, eflags, edi, esi, ebp, esp, edx, ecx, ebx, eax, eip, arg1, arg2
                                               
    mov eax, [esp+44]       ; The first argument, where to save 
    mov [eax+4], ebx        ;   
    mov [eax+8], ecx        ;   
    mov [eax+12], edx       ;   
    mov [eax+16], esi       ;   
    mov [eax+20], edi       ;

    mov ebx, [esp+36]       ; eax
    mov ecx, [esp+40]       ; eip
    mov esi, [esp+16]       ; ebp
    mov edi, [esp+4]        ; eflags
    mov edx, [esp+20]       ; esp
    add edx, 4              ; Remove the return value

    mov [eax], ebx          ; eax
    mov [eax+24], edx       ; esp
    mov [eax+28], esi       ; ebp
    mov [eax+32], ecx       ; eip
    mov [eax+36], edi       ; eflags

    pop ebx                 ; was pushed last
    mov [eax+40], ebx       ; cr3
    push ebx                ;
 
    mov eax, [esp+48]       ; eax points to "next" task obj
    mov ebx, [eax+4]        ;
    mov ecx, [eax+8]        ;                                     
    mov edx, [eax+12]       ;                                     
    mov esi, [eax+16]       ;                                     
    mov edi, [eax+20]       ;                                     
    mov ebp, [eax+28]       ;  

    push eax                ; save obj pointer
    mov eax, [eax+36]       ; eflags
    push eax                ; push onto stack
    popf                    ; pop into eflags registers
    pop eax                 ; restore obj pointer

    mov esp, [eax+24]       ; esp - restore "next" stack

    ; new stack
    push eax                ; save obj pointer
    mov eax, [eax+40]       ; cr3
    mov cr3, eax            ; restore cr3
    pop eax                 ; restore obj pointer

    push eax                ; save obj pointer
    mov eax, [eax+32]       ; eip - return address
    xchg [esp], eax         ; We do not have any more registers to use as tmp storage 
    mov eax, [eax]          ; eax

    ; iret should effectively call sti;
    sti;

    ret                     ; This ends all!


global switchTaskInterrupt
switchTaskInterrupt:
    cli;
                            ; arg2, arg1 (esp -= 8)
                            ; eip (esp -= 4)

    pusha                   ; ax,bx,cx,dx(16),sp,bp,si,di(16) (esp -= 32)
    pushf                   ; eflags (esp -= 4)
    
    mov eax, cr3                                         
    push eax                ; cr3 (esp -= 4)

    ; ESP  +  0 ,   4   ,  8 , 12 , 16 , 20 , 24 , 28 , 32 , 36 , 40 , 44 , 48
    ; Stack: cr3, eflags, edi, esi, ebp, esp, edx, ecx, ebx, eax, eip, arg1, arg2
                                               
    mov eax, [esp+44]       ; The first argument, where to save 
    mov [eax+4], ebx        ;   
    mov [eax+8], ecx        ;   
    mov [eax+12], edx       ;   
    mov [eax+16], esi       ;   
    mov [eax+20], edi       ;

    mov ebx, [esp+36]       ; eax
    mov ecx, [esp+40]       ; eip
    mov esi, [esp+16]       ; ebp
    mov edi, [esp+4]        ; eflags
    mov edx, [esp+20]       ; esp
    add edx, 4              ; Remove the return value

    mov [eax], ebx          ; eax
    mov [eax+24], edx       ; esp
    mov [eax+28], esi       ; ebp
    mov [eax+32], ecx       ; eip
    mov [eax+36], edi       ; eflags

    pop ebx                 ; was pushed last
    mov [eax+40], ebx       ; cr3
    push ebx                ;
 
    mov eax, [esp+48]       ; eax points to "next" task obj
    mov ebx, [eax+4]        ;
    mov ecx, [eax+8]        ;                                     
    mov edx, [eax+12]       ;                                     
    mov esi, [eax+16]       ;                                     
    mov edi, [eax+20]       ;                                     
    mov ebp, [eax+28]       ;  

    push eax                ; save obj pointer
    mov eax, [eax+36]       ; eflags
    push eax                ; push onto stack
    popf                    ; pop into eflags registers
    pop eax                 ; restore obj pointer

    mov esp, [eax+24]       ; esp - restore "next" stack

    ; new stack
    push eax                ; save obj pointer
    mov eax, [eax+40]       ; cr3
    mov cr3, eax            ; restore cr3
    pop eax                 ; restore obj pointer

    push eax                ; save obj pointer
    mov eax, [eax+32]       ; eip - return address
    xchg [esp], eax         ; We do not have any more registers to use as tmp storage 
    mov eax, [eax]          ; eax

    ; iret should effectively call sti;
    sti;

    ret                     ; This ends all!

