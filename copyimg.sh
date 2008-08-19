#!/bin/sh
mount /dev/fd0
cp kernel.bin /media/floppy
umount /media/floppy
