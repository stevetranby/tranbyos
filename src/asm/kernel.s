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

global switchTask
switchTask:
    pusha
    pushf
    
    mov eax, cr3                                         
    push eax
                                               
    mov eax, [esp+44]       ; The first argument, where to save 
    mov [eax+4], ebx        ;   
    mov [eax+8], ecx        ;   
    mov [eax+12], edx       ;   
    mov [eax+16], esi       ;   
    mov [eax+20], edi       ;   
    mov ebx, [esp+36]       ; EAX                                      
    mov ecx, [esp+40]       ; IP                                       
    mov edx, [esp+20]       ; ESP                                      
    add edx, 4              ; Remove the return value                  
                            ;  
    mov esi, [esp+16]       ; EBP                                      
    mov edi, [esp+4]        ; EFLAGS                                   
    mov [eax], ebx          ;   
    mov [eax+24], edx       ;   
    mov [eax+28], esi       ;   
    mov [eax+32], ecx       ;   
    mov [eax+36], edi       ;  
    pop ebx                 ; CR3                                      
                            ;  
    mov [eax+40], ebx 
    push ebx                ; Goodbye again                            
 
    mov eax, [esp+48]       ; Now it is the new object 
    mov ebx, [eax+4]        ;                                     
    mov ecx, [eax+8]        ;                                     
    mov edx, [eax+12]       ;                                     
    mov esi, [eax+16]       ;                                     
    mov edi, [eax+20]       ;                                     
    mov ebp, [eax+28]       ;                                     
    push eax                ;  
    mov eax, [eax+36]       ; EFLAGS                                   
    push eax                ; 
    popf                    ; pop eflags
    pop eax                 ; 
    mov esp, [eax+24]       ; ESP                                      
    push eax                ;  
    mov eax, [eax+44]       ; CR3 
    mov cr3, eax            ;  
    pop eax                 ;  
    push eax 
    mov eax, [eax+32]       ; EIP                                      
    xchg [esp], eax         ; We do not have any more registers to use as tmp storage 
    mov eax, [eax]          ; EAX                                           
    ret                     ; This ends all!                                           




;--------------------------------------------------------------------
; Paging

section text
global invalidate_page_tables
invalidate_page_tables:
    invlpg [eax]


switch_page_directory:
    push ebp
    mov ebp, esp
    mov eax, [esp+8]
    mov cr3, eax
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax


enable_pse:
    mov eax, cr4
    or eax, 0x00000010
    mov cr4, eax

; TODO:  Physical Address Extension (PAE)


section text
global loadPageDirectory
loadPageDirectory:
    push ebp
    mov ebp, esp
    mov eax, [esp+8] 
    mov cr3, eax 
    mov esp, ebp
    pop ebp
    ret

section text
global enablePaging
enablePaging:
    push ebp
    mov ebp, esp
    mov eax, cr0
    or  eax, 0x80000000
    mov cr0, eax
    mov esp, ebp
    pop ebp
    ret




