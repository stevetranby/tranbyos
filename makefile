OSNAME=tranbyos
CFLAGS=-g -Wall -Werror -fstrength-reduce -fomit-frame-pointer -finline-functions -fno-builtin -nostdinc -nostdlib -nostartfiles -nodefaultlibs -I./include
CC=gcc
SDIR=src
ODIR=obj
BDIR=bin
SFILES := $(wildcard $(SDIR)/*.c) #not used, as some recommend not using wildcards in case you add or copy a source file
OFILES := $(wildcard $(ODIR)/*.o) #not used, as some recommend not using wildcards in case you add or copy a source file

all: build
	@echo "All..."
	
build: link
	@echo "Building..."

link: compile
	@echo "Linking..."
	ld -T link.ld -o $(BDIR)/$(OSNAME).bin $(ODIR)/start.o $(ODIR)/main.o $(ODIR)/scrn.o $(ODIR)/gdt.o $(ODIR)/idt.o $(ODIR)/isrs.o $(ODIR)/irq.o $(ODIR)/timer.o $(ODIR)/kb.o $(ODIR)/mm.o $(ODIR)/hd.o $(ODIR)/io.o
	#ld -T link.ld -o $(BDIR)/$(OSNAME).bin $(ODIR)/*.o

compile: assemble
	@echo "Compiling..."
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

assemble: dirs
	@echo "\nAssembling...\n"
	nasm -f aout -o $(ODIR)/start.o $(SDIR)/start.asm

dirs:
	mkdir -p $(BDIR)
	mkdir -p $(ODIR)
	mkdir -p $(SDIR)
	@echo "\nDirectories Created.\n"

iso:
	mkisofs -o $(BDIR)/$(OSNAME).iso -b $(BDIR)/$(OSNAME).flp

disk: build
	@echo "\nInjecting Kernel Into Grub Image\n"

	@echo "\nCreating Grub Config File\n"
	@echo $(OSNAME)":\n\ttitle "$(OSNAME)"\n\troot (fd0)\n\tkernel /boot/"$(OSNAME)".bin\n" > grub.lst
	@echo 
	
	rm -rf tmp-loop
	mkdir tmp-loop 
	sudo mount -o loop -t vfat grub_disk.img tmp-loop 
	sudo cp $(BDIR)/$(OSNAME).bin tmp-loop/boot/$(OSNAME).bin
	sudo cp grub.lst tmp-loop/boot/menu.cfg
	sudo umount -f tmp-loop || exit
	
	@echo "Creating Blank QEMU Hard Drive Images For Testing"

	# QEMU version pre 0.14
	#qemu-img create -f qcow2 $(OSNAME)-hd-32mb.img 32MB
	#qemu-img create -f qcow2 $(OSNAME)-hd-64mb.img 64MB

	# QEMU version 0.14
	qemu-img create -f qcow2 $(OSNAME)-hd-32mb.img 32M
	qemu-img create -f qcow2 $(OSNAME)-hd-64mb.img 64M

#TODO: Add Scripts or Programs to Disk Image

test: build
	@echo "\nTesting...\n"	
	

run: disk
	@echo "\nRun in QEMU\n"	
	qemu -m 64 -hda $(OSNAME)-hd-32mb.img -hdb $(OSNAME)-hd-64mb.img -fda grub_disk.img 

#TODO: Should the directories bin/ and obj/ be removed, or just all the files inside?
#TODO: Should all the bin/* and obj/* files be removed, or should they just be overwritten each time?
clean: 
	@echo Files Cleaned.
	rm -f $(BDIR)/*
	rm -f $(ODIR)/*.o

	
