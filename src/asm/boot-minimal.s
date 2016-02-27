; Utilize 32-bit x86
[BITS 16]

org 0x7c00    ;BIOS loaded at 0x7c00

bits 16       ; We are still in 16 bit real mode

Start:
    cli      ; clear all interrupts
    hlt      ; halt the system

    times 510 - ($-$$) db 0  ;We have to be 512 bytes.Clear rest of bytes with 0
    ddw 0xAA55               ;Boot signature

 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

 _start:
    ; Write kernel here. It might be good to load a new GDT.
    mov edi, 0xB8000
    mov esi, string
    mov ah, 0x0F
    .displaying:
    lodsb
    stosw
    or al, al
    jnz .displaying
    jmp short $

string:
    db "Hello world!", 0
