org 0x7c00    ;BIOS loaded at 0x7c00

bits 16       ; We are still in 16 bit real mode

 Start:
 cli      ; clear all interrupts
 hlt      ; halt the system

 times 510 - ($-$$) db 0  ;We have to be 512 bytes.Clear rest of bytes with 0

 ddw 0xAA55               ;Boot signature