
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


Try it out TranbyOS (tested on macOS, should work on any host where QEMU runs)

- http://www.qemu-project.org/download
- https://github.com/stevetranby/tranbyos/blob/master/tools/grub_disk_backup.img
- https://github.com/stevetranby/tranbyos/blob/master/tools/tranbyos-hd-32mb.img

## Used run commands during testing:
```
qemu-system-i386 -m 64 -rtc base=localtime,clock=host,driftfix=slew -hda tranbyos-hd-32mb.img -hdb tranbyos-hd-32mb.img -vga std -serial stdio -fda grub_disk_backup.img
```
Simpler Method that should still work: (try -hda if -hdb doesn't work)
```
qemu-system-i386  -m 64 -hdb tranbyos-hd-32mb.img -vga std -serial stdio -fda grub_disk_backup.img
```
Press a key until asks for [SPACE]

Should see debug logs in stdout (assume run from a terminal). 

If it succeeds through all tests should see black square mouse cursor.


Building Cross Compiler
-------------------------

Why? Because the default compilers include standard libraries and other optimizations that your base OS won't have until you add support for them. See: reference to OSDev about why X-Compile

# Building with Ubuntu 14.04 Server

* could look into using crosstool-ng


Using crosstool-ng
-------------------

I'm going to only use linux to develop for now, so use a virtual machine if on OSX or Windows. I previously created a cross-compiler on mac by hand, but with crosstool-ng things are now much simpler.

I'm going to build for i{3,5,6}86 to start and then look at building for amd64 and arm (Raspberry Pi and Nvidia's Tegra2 Jeston TK1).


Tools
-----

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

[Google Doc on How To Build OS](https://docs.google.com/document/d/17-3cOyXNgPmjh05qogS-h6hKn6WDllWs0xOVhEWvj9I/edit?usp=sharing)

Resources
------------------------------------------

- [http://www.osdever.net/tutorials/view/getting-started-in-os-development](http://www.osdever.net/tutorials/view/getting-started-in-os-development)
- [http://mikeos.berlios.de/](http://mikeos.berlios.de/)
- http://www.jamesmolloy.co.uk/tutorial_html/index.html
- http://wiki.osdev.org/Main_Page
- http://wiki.osdev.org/Bare_bones

SETUP OSX Lion
------------------------------------------
= Cross-Compiler =
brew install binutils
brew install gcc (pulls in deps: mpfr, libmpc, isl, cloog)
brew install gmp mpc libmpc

### Follow instructions

http://wiki.osdev.org/GCC_Cross-Compiler

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
