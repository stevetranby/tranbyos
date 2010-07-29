CFLAGS=-g -Wall -Werror -fstrength-reduce -fomit-frame-pointer -finline-functions -fno-builtin -nostdinc -nostdlib -nostartfiles -nodefaultlibs -I./include
GCC=gcc
LINKER=ld
BD=build

all: build
	echo "All..."
	
build: link
	echo "Building..."

link: compile	
	echo "Linking..."
	#$(LINKER) -T link.ld -o kernel.bin $(BD)/start.o $(BD)/main.o $(BD)/scrn.o $(BD)/gdt.o $(BD)/idt.o $(BD)/isrs.o $(BD)/irq.o $(BD)/timer.o $(BD)/kb.o $(BD)/mm.o $(BD)/hd.o $(BD)/io.o
	$(LINKER) -T link.ld -o kernel.bin $(BD)/*.o

compile: assemble
	echo "Compiling..."
	$(GCC) $(CFLAGS) -c -o $(BD)/main.o main.c
	$(GCC) $(CFLAGS) -c -o $(BD)/scrn.o scrn.c
	$(GCC) $(CFLAGS) -c -o $(BD)/gdt.o gdt.c
	$(GCC) $(CFLAGS) -c -o $(BD)/idt.o idt.c
	$(GCC) $(CFLAGS) -c -o $(BD)/isrs.o isrs.c
	$(GCC) $(CFLAGS) -c -o $(BD)/irq.o irq.c
	$(GCC) $(CFLAGS) -c -o $(BD)/timer.o timer.c
	$(GCC) $(CFLAGS) -c -o $(BD)/kb.o kb.c
	$(GCC) $(CFLAGS) -c -o $(BD)/mm.o mm.c
	$(GCC) $(CFLAGS) -c -o $(BD)/hd.o hd.c
	$(GCC) $(CFLAGS) -c -o $(BD)/io.o io.c

assemble:
	echo "Assembling..."
	nasm -f aout -o $(BD)/start.o start.asm

test: build
	echo "Testing"

run: build
	echo "Running"
	
clean:
	echo "Cleaning..."
	rm $(BD)/*.o

	
