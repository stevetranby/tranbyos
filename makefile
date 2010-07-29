CFLAGS=-g -Wall -Werror -fstrength-reduce -fomit-frame-pointer -finline-functions -fno-builtin -nostdinc -nostdlib -nostartfiles -nodefaultlibs -I./include
CC=gcc
SDIR=src
ODIR=obj
BDIR=bin
SRC_FILES := $(wildcard)
OBJ_FILES := $(wildcard)

all: build
	echo "All..."
	
build: link
	echo "Building..."

link: compile	
	echo "Linking..."
	ld -T link.ld -o $(BDIR)/kernel.bin $(ODIR)/start.o $(ODIR)/main.o $(ODIR)/scrn.o $(ODIR)/gdt.o $(ODIR)/idt.o $(ODIR)/isrs.o $(ODIR)/irq.o $(ODIR)/timer.o $(ODIR)/kb.o $(ODIR)/mm.o $(ODIR)/hd.o $(ODIR)/io.o
	#ld -T link.ld -o $(BDIR)/kernel.bin $(ODIR)/*.o

compile: assemble
	echo "Compiling..."
	$(CC) $(CFLAGS) -c -o $(ODIR)/main.o $(SDIR)/main.c
	$(CC) $(CFLAGS) -c -o $(ODIR)/scrn.o $(SDIR)/scrn.c
	$(CC) $(CFLAGS) -c -o $(ODIR)/gdt.o $(SDIR)/gdt.c
	$(CC) $(CFLAGS) -c -o $(ODIR)/idt.o $(SDIR)/idt.c
	$(CC) $(CFLAGS) -c -o $(ODIR)/isrs.o $(SDIR)/isrs.c
	$(CC) $(CFLAGS) -c -o $(ODIR)/irq.o $(SDIR)/irq.c
	$(CC) $(CFLAGS) -c -o $(ODIR)/timer.o $(SDIR)/timer.c
	$(CC) $(CFLAGS) -c -o $(ODIR)/kb.o $(SDIR)/kb.c
	$(CC) $(CFLAGS) -c -o $(ODIR)/mm.o $(SDIR)/mm.c
	$(CC) $(CFLAGS) -c -o $(ODIR)/hd.o $(SDIR)/hd.c
	$(CC) $(CFLAGS) -c -o $(ODIR)/io.o $(SDIR)/io.c

assemble:
	echo "Assembling..."
	nasm -f aout -o $(ODIR)/start.o start.asm

makeiso:

makeflp: 
	mkdosfs -C $(BDIR)/diskimage.flp 1440

makefloppy:

#TODO: Add Scripts or Programs to Disk Image

test: build
	echo "Testing"

run: build
	echo "Running"
	
clean:
	echo "Cleaning..."
	rm $(BDIR)/*
	rm $(ODIR)/*.o

	
