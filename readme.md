                                                                                
    _/_/_/_/_/                                _/                    _/_/      _/_/_/   
       _/      _/  _/_/    _/_/_/  _/_/_/    _/_/_/    _/    _/  _/    _/  _/          
      _/      _/_/      _/    _/  _/    _/  _/    _/  _/    _/  _/    _/    _/_/       
     _/      _/        _/    _/  _/    _/  _/    _/  _/    _/  _/    _/        _/      
    _/      _/          _/_/_/  _/    _/  _/_/_/      _/_/_/    _/_/    _/_/_/         
                                                         _/                            
                                                    _/_/         
    

Tranby OS 
===========

Developing x86 simple operating system to learn the fundamentals of the boot process, device interaction, among other things ...



Setup Environment
-------------------

Most linux distros will have binutils, make, gcc, and ld already installed.

 - binutils (standard )
 - make (build tool)
 - gcc (gnu compiler)
 - ld (linker)
 - nasm (assembler)
 - qemu (emulator)


Build and Run
--------------------

make build
- Creates a binary kernel from assembling, compiling, and linking.

make disk
- Modifies standard GRUB disk image to include the kernel and configuration
- Creates the QEMU hard disk images

make run 
- Loads QEMU with GRUB disk image and hard disk images


OS Details
---------------------

*Multiboot Header*
Using GRUB as the bootloader, since a bootloader is almost as much work as an OS kernel or more, I am using GRUB as suggested by many of the c/c++ kernel development guides.

*Simple Console Library*

*Interrupts and IRQs*
Currently these are mostly from Bran's Tutorial

*Keyboard Support*

*Timer Interrupt*
100Hz is the current interrupt frequency

*Simple Hard Disk ATA Driver*
Currently reads and writes data successfully.

_I'll write more as I have time to work on this_


Extensive How-To In Progress
------------------------------------------

[Google Doc on How To Build OS](https://docs.google.com/document/edit?id=17-3cOyXNgPmjh05qogS-h6hKn6WDllWs0xOVhEWvj9I&hl=en)

Resources
------------------------------------------

- [http://www.osdever.net/tutorials/view/getting-started-in-os-development](http://www.osdever.net/tutorials/view/getting-started-in-os-development)
- [http://mikeos.berlios.de/](http://mikeos.berlios.de/)
- http://www.jamesmolloy.co.uk/tutorial_html/index.html
- http://wiki.osdev.org/Main_Page
- http://wiki.osdev.org/Bare_bones


OLD README
------------------------------------------

GDT [Global Descriptor Table]

Multitasking:
http://www.osdever.net/tutorials/multitasking.php

Tips/Problems
------------------------------------------

Prob: 
  undefined reference to '__stack_chk_fail'

Desc: 
  This problem occurs when the gcc compiler thinks you have a possible buffer
  overflow, or the code cannot be statically proven to be safe to such a bug.

Soln: 
  add -fno-stack-protector to the CFLAGS as an argument to gcc
  [http://hackinglinux.blogspot.com/2006/11/resolving-stackchkfail-error.html](http://hackinglinux.blogspot.com/2006/11/resolving-stackchkfail-error.html)

-------------

Prob:
  
Desc:
Soln:
