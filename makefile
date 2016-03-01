OSNAME=tranbyos
CFLAGS_BASE= -O0 -g -m32 -Wall -Werror -Wno-unused-function -fstrength-reduce -fomit-frame-pointer -finline-functions -nostdlib -nostartfiles -nodefaultlibs -ffreestanding
CFLAGS=-std=c11 $(CFLAGS_BASE) -m32 -I./src/include
CXXFLAGS=-std=c++14 $(CFLAGS_BASE) -fno-exceptions -fno-rtti -m32 -I./src/include
CXX64FLAGS=-std=c++14 $(CFLAGS_BASE) -fno-exceptions -fno-rtti -m64 -I./src/include

#CC=gcc
CC=x86_64-elf-gcc
CXX=x86_64-elf-g++
LD=x86_64-elf-ld

BUILD_DIR=tools
SRC=src
OBJ_DIR=obj
BIN_DIR=bin
ASM_DIR=$(SRC)/asm
INC_DIR=$(SRC)/include

# these are not used, as some recommend not using wildcards
# in case you add or copy a source file.
# Note(steve): Could reconsider
#SFILES := $(wildcard $(SRC)/*.c)
#OFILES := $(wildcard $(OBJ_DIR)/*.o)

# QEMU version 0.14
QIMG="qemu-img"
QEMU="qemu-system-i386"
# Q (OSX App works best for Lion as of May2012)
# http://www.kju-app.org/
#QIMG=/Applications/Q.app/Contents/MacOS/qemu-img
#QEMU=/Applications/Q.app/Contents/MacOS/i386-softmmu.app/Contents/MacOS/i386-softmmu


all: build
	@echo "All..."

build: link
	@echo "Building..."

link: compile assemble
	@echo "Linking..."
	$(LD) -m elf_i386 -T $(BUILD_DIR)/link.ld -o $(BIN_DIR)/$(OSNAME).bin $(OBJ_DIR)/start.o \
	$(OBJ_DIR)/main.o $(OBJ_DIR)/scrn.o $(OBJ_DIR)/gdt.o \
	$(OBJ_DIR)/isrs.o $(OBJ_DIR)/timer.o $(OBJ_DIR)/kb.o \
	$(OBJ_DIR)/mm.o $(OBJ_DIR)/hd.o $(OBJ_DIR)/io.o $(OBJ_DIR)/vga.o

	@#$(ld) -T link.ld -o $(BIN_DIR)/$(OSNAME).bin $(OBJ_DIR)/*.o

compile: dirs
	@echo "Compiling..."
	$(CC) $(CFLAGS) -c -o $(OBJ_DIR)/main.o $(SRC)/main.c
	$(CC) $(CFLAGS) -c -o $(OBJ_DIR)/scrn.o $(SRC)/scrn.c
	$(CC) $(CFLAGS) -c -o $(OBJ_DIR)/gdt.o $(SRC)/gdt.c
	$(CC) $(CFLAGS) -c -o $(OBJ_DIR)/isrs.o $(SRC)/isrs.c
	$(CC) $(CFLAGS) -c -o $(OBJ_DIR)/timer.o $(SRC)/timer.c
	$(CC) $(CFLAGS) -c -o $(OBJ_DIR)/kb.o $(SRC)/kb.c
	$(CC) $(CFLAGS) -c -o $(OBJ_DIR)/mm.o $(SRC)/mm.c
	$(CC) $(CFLAGS) -c -o $(OBJ_DIR)/hd.o $(SRC)/hd.c
	$(CC) $(CFLAGS) -c -o $(OBJ_DIR)/io.o $(SRC)/io.c
	$(CC) $(CFLAGS) -c -o $(OBJ_DIR)/vga.o $(SRC)/vga.c

	@#$(CXX) $(CXXFLAGS) -c -o $(OBJ_DIR)/test.o $(SRC)/test.cpp

# currently working on
assemble: dirs
	@echo "\nAssembling...\n"
	nasm -f elf -o $(OBJ_DIR)/start.o $(ASM_DIR)/start.s
	#nasm -f aout -o $(OBJ_DIR)/start.o $(SRC)/start.s

dirs:
	mkdir -p $(BIN_DIR)
	mkdir -p $(OBJ_DIR)
	@echo "\nDirectories Created.\n"

iso:
	mkisofs -o $(BIN_DIR)/$(OSNAME).iso -b $(BIN_DIR)/$(OSNAME).flp
	#grub-mkrescue --output=$(BUILD_DIR)tranbyos.iso $(BUILD_DIR)/boot/grub

copykernel: build
	@echo "\nInjecting Kernel Into Grub Image"
	$(BUILD_DIR)/build-copykernel-grub.sh

disks:
	@echo "\nCreating Blank QEMU Hard Drive Images For Testing"
	$(QIMG) create -f qcow2 $(BUILD_DIR)/$(OSNAME)-hd-32mb.img 32M
	$(QIMG) create -f qcow2 $(BUILD_DIR)/$(OSNAME)-hd-64mb.img 64M


#TODO: Add Scripts or Programs to Disk Image
test: build
	@echo "\nTesting...\n"


# others
# - https://github.com/zhiayang/mx/blob/develop/tools/createdisk.sh
# - 
#
# -vga [std|cirrus|vmware|qxl|xenfb|tcx|cg3|virtio|none]
# => VESA VBE virtual graphic card (std 1024x768, cirrus 4096x4096??, vmware fastest if can setup)
# -rtc [base=utc|localtime|date][,clock=host|vm][,driftfix=none|slew]
run: copykernel disks
	@echo "\nRun in QEMU"
	# Use GRUB for multiboot
	#$(QEMU) -m 32 -rtc base=localtime,clock=host,driftfix=slew -hda $(BUILD_DIR)/$(OSNAME)-hd-32mb.img -hdb $(BUILD_DIR)/$(OSNAME)-hd-64mb.img -fda $(BUILD_DIR)/grub_disk.img -vga vmware -serial stdio
	# Use QEMU directly as multiboot loader
	$(QEMU) -m 32 -rtc base=localtime,clock=host,driftfix=slew -hda $(BUILD_DIR)/$(OSNAME)-hd-32mb.img -hdb $(BUILD_DIR)/$(OSNAME)-hd-64mb.img -vga std -serial stdio -fda $(BUILD_DIR)/grub_disk.img
	# -usb, -usbdevice mouse

riso:
	mkdir -p $(BUILD_DIR)/tmp/boot/grub
	cp $(BIN_DIR)/$(OSNAME).bin $(BUILD_DIR)/tmp/boot/grub
	mkisofs -J -o $(BUILD_DIR)/tranbyos.iso $(BUILD_DIR)/tmp
	grub-mkrescue --modules="iso9660 biosdisk multiboot" --output=$(BUILD_DIR)/tranbyos.iso $(BUILD_DIR)/tmp
	$(QEMU) -m 32 -rtc base=localtime,clock=host,driftfix=slew -hda $(BUILD_DIR)/$(OSNAME)-hd-32mb.img -hdb $(BUILD_DIR)/$(OSNAME)-hd-64mb.img -vga std -serial stdio -cdrom $(BUILD_DIR)/tranbyos.iso

#TODO: Should the directories bin/ and obj/ be removed, or just all the files inside?
#TODO: Should all the bin/* and obj/* files be removed, or should they just be overwritten each time?
clean:
	@echo Files Cleaned.
	@echo $(PATH)
	rm -f $(BUILD_DIR)/$(OSNAME)-hd-32mb.img
	rm -f $(BUILD_DIR)/$(OSNAME)-hd-64mb.img
	rm -f $(BIN_DIR)/*
	rm -f $(OBJ_DIR)/*.o


deps:
	$(CC) -MMD -MF $out.d $(CFLAGS)
