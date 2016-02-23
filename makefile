OSNAME=tranbyos

CFLAGS=-g -m32 -Wall -Werror -fstrength-reduce -fomit-frame-pointer -finline-functions -fno-builtin -nostdinc -nostdlib -nostartfiles -nodefaultlibs -std=c99 -I./include
CXXFLAGS=-g -m32 -Wall -Werror -fstrength-reduce -fomit-frame-pointer -finline-functions -fno-builtin -nostdinc -nostdlib -nostartfiles -nodefaultlibs -ffreestanding -O2 -Wextra -fno-exceptions -fno-rtti -I./include

#CC=gcc
CC=i386-elf-gcc
CXX=i386-elf-g++
LD=i386-elf-ld

# QEMU version 0.14
QIMG=qemu-img
QEMU=qemu-system-i386
# Q (OSX App works best for Lion as of May2012)
# http://www.kju-app.org/
#QIMG=/Applications/Q.app/Contents/MacOS/qemu-img
#QEMU=/Applications/Q.app/Contents/MacOS/i386-softmmu.app/Contents/MacOS/i386-softmmu

SDIR=src
ODIR=obj
BDIR=bin

# these are not used, as some recommend not using wildcards 
# in case you add or copy a source file.
# Note(steve): Could reconsider
#SFILES := $(wildcard $(SDIR)/*.c) 
#OFILES := $(wildcard $(ODIR)/*.o) 


all: build
	@echo "All..."

build: link
	@echo "Building..."

link: compile
	@echo "Linking..."
	$(LD) -T link.ld -o $(BDIR)/$(OSNAME).bin $(ODIR)/start.o $(ODIR)/main.o $(ODIR)/scrn.o $(ODIR)/gdt.o $(ODIR)/idt.o $(ODIR)/isrs.o $(ODIR)/irq.o $(ODIR)/timer.o $(ODIR)/kb.o $(ODIR)/mm.o $(ODIR)/hd.o $(ODIR)/io.o $(ODIR)/vga.o 
	@#$(ld) -T link.ld -o $(BDIR)/$(OSNAME).bin $(ODIR)/*.o

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
	$(CC) $(CFLAGS) -c -o $(ODIR)/vga.o $(SDIR)/vga.c

	@#$(CXX) $(CXXFLAGS) -c -o $(ODIR)/test.o $(SDIR)/test.cpp

assemble: dirs
	@echo "\nAssembling...\n"
	#nasm -f aout -o $(ODIR)/start.o $(SDIR)/start.asm
	nasm -f elf -o $(ODIR)/start.o $(SDIR)/start.asm

dirs:
	mkdir -p $(BDIR)
	mkdir -p $(ODIR)
	mkdir -p $(SDIR)
	@echo "\nDirectories Created.\n"

iso:
	mkisofs -o $(BDIR)/$(OSNAME).iso -b $(BDIR)/$(OSNAME).flp

copykernel: build
	@echo "\nInjecting Kernel Into Grub Image\n"
	@echo "\nCreating Grub Config File\n"
	@echo $(OSNAME)":\n\ttitle "$(OSNAME)"\n\troot (fd0)\n\tset gfxmode=auto\n\tkernel /boot/"$(OSNAME)".bin\n" > grub.lst
	@echo

	# Linux Disk Mount
	#rm -rf tmp-loop
	#mkdir tmp-loop
	#sudo mount -o loop -t vfat grub_disk.img tmp-loop
	#sudo cp $(BDIR)/$(OSNAME).bin tmp-loop/boot/$(OSNAME).bin
	#sudo cp grub.lst tmp-loop/boot/menu.cfg
	#sudo umount -f tmp-loop || exit

	# OSX Lion Disk Mount
	hdiutil attach grub_disk.img
	cp $(BDIR)/$(OSNAME).bin /Volumes/GRUB/boot/$(OSNAME).bin
	cp grub.lst /Volumes/GRUB/boot/menu.cfg
	#hdiutil detach /Volumes/GRUB


disks:
	@echo "Creating Blank QEMU Hard Drive Images For Testing"
	$(QIMG) create -f qcow2 $(OSNAME)-hd-32mb.img 32M
	$(QIMG) create -f qcow2 $(OSNAME)-hd-64mb.img 64M


#TODO: Add Scripts or Programs to Disk Image
test: build
	@echo "\nTesting...\n"


# -vga [std|cirrus|vmware|qxl|xenfb|tcx|cg3|virtio|none]
# => VESA VBE virtual graphic card (std 1024x768, cirrus 4096x4096??, vmware fastest if can setup)
run: copykernel disks
	@echo "\nRun in QEMU\n"
	$(QEMU) -m 32 -hda $(OSNAME)-hd-32mb.img -hdb $(OSNAME)-hd-64mb.img -fda grub_disk.img -vga vmware -serial stdio


#TODO: Should the directories bin/ and obj/ be removed, or just all the files inside?
#TODO: Should all the bin/* and obj/* files be removed, or should they just be overwritten each time?
clean:
	@echo Files Cleaned.
	@echo $(PATH)
	rm -f $(OSNAME)-hd-32mb.img
	rm -f $(OSNAME)-hd-64mb.img
	rm -f $(BDIR)/*
	rm -f $(ODIR)/*.o


