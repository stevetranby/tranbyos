; Utilize 32-bit x86
[BITS 32]

; TODO: can we move this into C code?


; From: http:// ??????

section .bss ; $Revision: 1.6 $
vendor_id       resd	12	;reserve 12 bytes of memory
version		resd	4
features	resd	4
i		resd	4
curfeat		resd	4


section .text
    global _start                       ;must be declared for linker (ld)
names	db	'FPU  VME  DE   PSE  TSC  MSR  PAE  MCE  CX8  APIC RESV SEP  MTRR PGE  MCA  CMOV PAT PSE3 PSN  CLFS RESV DS   ACPI MMX FXSR SSE  SSE2 SS   HTT  TM   RESV PBE '

_start:					;tell linker entry point

mov eax,0
cpuid
mov [vendor_id],ebx
mov [vendor_id+4],edx
mov [vendor_id+8],ecx

        ;;syscall(SYS_write, 0, str, sizeof(str)-1)
        mov     edx,12  ;message length
        mov     ecx,vendor_id   ;message to write (msg is a pointer to the start of the string

        mov     ebx,1   ;file descriptor (stdout)
        mov     eax,4   ;system call number (sys_write)
        int     0x80    ;call kernel


mov eax,1
cpuid
mov [version],eax
mov [features],edx
;mov [100000f0h],ebx ;break program for debugging

;; loop
mov	eax,00000001h
mov	[curfeat],eax
mov     eax,-1
mov     [i],eax
.loop:
;;--i
	mov     eax,[i] 
	inc eax         ;(i++)
	cmp     eax,31
	jz     .quitloop   ;quit loop if reached limit
	mov     [i],eax    ;put updated value on stack

;get current feature
        mov ebx,[curfeat]
;test for feature - if feature exists ebx is non zero
	and ebx,[features]
;left shift to test for next feature (will be used in next iteration of loop)
	mov eax,[curfeat]
        shl eax,1
        mov [curfeat],eax

;jump if feaure not exist
cmp ebx,0 
jz .loop ;check if zero flag is set - if it is it means that the feature didn't exist so we don't want to print anything out
;;otherwise this feature must exist lets print it out...
        mov     eax,[i] ;get value from stack           0x080480bf
        mov     edx,5   ;message length
        mov     ecx,names     ;message to write (msg is a pointer to the start of the string
times 5	add	ecx,eax
	
        mov     ebx,1   ;file descriptor (stdout)
        mov     eax,4   ;system call number (sys_write) 0x080480b7
        int     0x80    ;call kernel                    0x080480be

	jmp .loop ; unconditional jump
;}
.quitloop:

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
mov	eax,1	;system call number (sys_exit)
	mov	ebx,0	;exit status 0 (if not used is 1 as set before) "echo $?" to check
	int	0x80	;call kernel
