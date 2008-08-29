CFLAGS = -g -Wall -Werror -fno-stack-protector -fstrength-reduce -fomit-frame-pointer -finline-functions -fno-builtin -nostdinc -nostdlib -nostartfiles -nodefaultlibs -I./include

all:
	nasm -f elf -o start.o start.asm
	
	gcc $(CFLAGS) -c -o main.o main.c
	gcc $(CFLAGS) -c -o scrn.o scrn.c
	gcc $(CFLAGS) -c -o gdt.o gdt.c
	gcc $(CFLAGS) -c -o idt.o idt.c
	gcc $(CFLAGS) -c -o isrs.o isrs.c
	gcc $(CFLAGS) -c -o irq.o irq.c
	gcc $(CFLAGS) -c -o timer.o timer.c
	gcc $(CFLAGS) -c -o kb.o kb.c
	gcc $(CFLAGS) -c -o mm.o mm.c
	gcc $(CFLAGS) -c -o hd.o hd.c
	
	ld -T link.ld -r -o kernel.o start.o main.o scrn.o gdt.o idt.o isrs.o irq.o timer.o kb.o mm.o hd.o
	ld -T link.ld -o kernel.bin start.o main.o scrn.o gdt.o idt.o isrs.o irq.o timer.o kb.o mm.o hd.o
	
	./copyimg.sh

clean:
	rm *.o

	