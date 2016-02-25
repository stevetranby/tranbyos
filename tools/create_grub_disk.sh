#!/usr/bin/env bash

OSNAME=tranbyos
BUILD_DIR=tools
echo "${OSNAME}:\n\ttitle ${OSNAME}\n\troot (fd0)\n\tset gfxmode=auto\n\tkernel /boot/${OSNAME}.bin\n"
echo "${OSNAME}:\n\ttitle ${OSNAME}\n\troot (fd0)\n\tset gfxmode=auto\n\tkernel /boot/${OSNAME}.bin\n" > "${BUILD_DIR}/grub.lst"