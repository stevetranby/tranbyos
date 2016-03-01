; Save for later, or testing

GLOBAL _jump_usermode ;you may need to remove this _ to work right.. 
EXTERN _test_user_function
_jump_usermode:
     mov ax,0x23
     mov ds,ax
     mov es,ax 
     mov fs,ax 
     mov gs,ax ;we don't need to worry about SS. it's handled by iret                
     mov eax,esp
     push 0x23 ;user data segment with bottom 2 bits set for ring 3
     push eax ;push our current stack just for the heck of it
     pushf
     push 0x1B; ;user data segment with bottom 2 bits set for ring 3
     push _test_user_function ;may need to remove the _ for this to work right 
     iret ;is there really a different way to make this?
;end