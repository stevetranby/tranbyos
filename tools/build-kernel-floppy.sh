#!/bin/sh
mount /dev/fd0
cp bin/tranbyos.bin /media/floppy
umount /media/floppy
