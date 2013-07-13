#!/bin/bash


# --- using: LLVM-GCC 4.2 or GCC 4 (for 64-bit) ---


# make an empty directory to build in, and go to it

mkdir -p obj
rm -f obj/*
cd obj


# choose compiler and set options

# if llvm-gcc is available:
COMPILER=/Developer/usr/bin/llvm-gcc
LINKER=/Developer/usr/bin/llvm-gcc
OPTI="-O4 -ffast-math"
# or use gcc:
#COMPILER=gcc
#LINKER=gcc
#OPTI="-O3 -ffast-math"

LANG="-x c -ansi -std=iso9899:199409 -pedantic"
WARN="-Wall -Wextra -Wcast-align -Wwrite-strings -Wpointer-arith -Wredundant-decls -Wdisabled-optimization"
CPU="-arch x86_64"
ARCH=""

COMPILE_OPTIONS="-c $LANG $OPTI $CPU $ARCH $WARN -Isrc"
LINK_OPTIONS=$CPU


# compile and link

echo
$COMPILER --version

$COMPILER $COMPILE_OPTIONS ../src/*.c

$LINKER $LINK_OPTIONS -o minilight-c *.o


# move executable and return from build directory

mv minilight-c ..
cd ..
rm obj/*


exit
