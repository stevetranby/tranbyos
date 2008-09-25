TARGET=i586-elf
CROSSPATH=/usr/cross/bin
CFLAGS=-g -Wall -Werror -fstrength-reduce -fomit-frame-pointer -finline-functions -fno-builtin -nostdinc -nostdlib -nostartfiles -nodefaultlibs -I./include
GCC=$(CROSSPATH)/$(TARGET)-gcc
LINKER=$(CROSSPATH)/$(TARGET)-ld

all:
	nasm -f elf -o start.o start.asm
	
	$(GCC) $(CFLAGS) -c -o main.o main.c
	$(GCC) $(CFLAGS) -c -o scrn.o scrn.c
	$(GCC) $(CFLAGS) -c -o gdt.o gdt.c
	$(GCC) $(CFLAGS) -c -o idt.o idt.c
	$(GCC) $(CFLAGS) -c -o isrs.o isrs.c
	$(GCC) $(CFLAGS) -c -o irq.o irq.c
	$(GCC) $(CFLAGS) -c -o timer.o timer.c
	$(GCC) $(CFLAGS) -c -o kb.o kb.c
	$(GCC) $(CFLAGS) -c -o mm.o mm.c
	$(GCC) $(CFLAGS) -c -o hd.o hd.c
	$(GCC) $(CFLAGS) -c -o io.o io.c
	
	$(LINKER) -T link.ld -r -o kernel.o start.o main.o scrn.o gdt.o idt.o isrs.o irq.o timer.o kb.o mm.o hd.o io.o
	$(LINKER) -T link.ld -o kernel.bin start.o main.o scrn.o gdt.o idt.o isrs.o irq.o timer.o kb.o mm.o hd.o io.o
	
clean:
	rm *.o

	