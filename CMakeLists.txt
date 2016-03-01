cmake_minimum_required(VERSION 3.3)
project(tranbyos)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    src/asm/start.s
    src/include/multiboot.h
    src/include/system.h
    src/fs.c
    src/gdt.c
    src/gui.c
    src/hd.c
    src/io.c
    src/isrs.c
    src/kb.c
    src/main.c
    src/mm.c
    src/scrn.c
    src/string.c
    src/timer.c
    src/vesa.c
    src/vga.c
    tests/test-hd.c)

add_executable(tranbyos ${SOURCE_FILES})