#! /bin/sh

OSNAME="tranbyos"

PROJ_DIR=".."
BUILD_DIR="." #this, or SCRIPT_NAME?
SRC="${PROJ_DIR}/src"
OBJ_DIR="${PROJ_DIR}/obj"
BIN_DIR="${PROJ_DIR}/bin"
ASM_DIR="${SRC}/asm"
INC_DIR="${SRC}/include"

echo "$OSNAME $BUILD_DIR $SRC $OBJ_DIR $BIN_DIR $ASM_DIR $INC_DIR"
echo "grub.cfg auto-create disabled."

# TODO: init grub.cfg if doesn't exist
#OSNAME=tranbyos
#BUILD_DIR=tools
# echo "${OSNAME}:\n\ttitle ${OSNAME}\n\troot (fd0)\n\tset gfxmode=auto\n\tkernel /boot/${OSNAME}.bin\n" > "${BUILD_DIR}/grub.cfg"

##############################################
# Copy Into Grub Image
has_command() { 
	cmd = $1 || ""
	if command -v $1 >/dev/null 2>&1; then
	else
		echo >&2 "I require $1 but it's not installed. Aborting."; exit 1; 
	fi
}

copy_grub()
{
	if has_command "hdiutil"; then
		echo "WE HAVE hdiutil, use it"
	elif has_command "uname"; then
		echo "WE HAVE uname, use it"
	fi

uname -v 

	echo "copying kernel into grub image along with configuration"
	# TODO: error checking
	# TODO: make conditional on OS

	# TODO: if [-z hdiutil]

	# if (HOST = OSX)
	# OSX Lion Disk Mount
	hdiutil attach ${BUILD_DIR}/grub_disk.img
	cp ${BIN_DIR}/${OSNAME}.bin /Volumes/GRUB/boot/${OSNAME}.bin
	cp ${BUILD_DIR}/grub.cfg /Volumes/GRUB/boot/menu.cfg
	hdiutil detach /Volumes/GRUB

	# if (HOST = Linux)
	# Linux Disk Mount
	#rm -rf tmp-loop
	#mkdir tmp-loop
	#sudo mount -o loop -t vfat grub_disk.img tmp-loop
	#sudo cp ${BIN_DIR}/${OSNAME}.bin tmp-loop/boot/${OSNAME}.bin
	#sudo cp grub.cfg tmp-loop/boot/menu.cfg
	#sudo umount -f tmp-loop || exit
}

##############################################
# Copy Pure Kernel onto Floppy
copy_floppy()
{
	# TODO: error checking
	mount /dev/fd0
	cp ${BIN_DIR}/tranbyos.bin /media/floppy
	umount /media/floppy
}

copy_grub

# TEST building command router

cmd_setup_grub() 
{
	echo "$0 : $1, $2, $3"
}

cmd_setup_floppy() 
{
	echo "$0 : $1, $2, $3"
}

cmd_setup_cdrom() 
{
	echo "$0 : $1, $2, $3"
}

run_cmd()
{
	echo "run_cmd:"
	echo "$0 : $1, $2, $3"
	if [ $1 == "grub" ]; then
		cmd_setup_grub
	elif [ $1 == "floppy" ]; then
		cmd_setup_floppy
	elif [ $1 == "cdrom" ]; then
		cmd_setup_cdrom
	else 
		echo "command '$1' doesn't exist"
	fi
}

if [ -z "$1" ]; then
     echo "TODO: print usage"
 else
     echo "running command: $1"
	run_cmd $1
fi

