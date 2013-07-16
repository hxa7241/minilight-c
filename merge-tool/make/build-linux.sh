#!/bin/bash


# --- using: Clang or GCC 4 ---


# make an empty directory to build in and go to it

mkdir -p obj
rm -f obj/*
cd obj


# choose compiler and set options

# Clang produces a significantly faster executable
if which clang
then
   COMPILER=clang
   LINKER=llvm-ld
   
   OPTI="-O4 -ffast-math -emit-llvm"
   #LINK_OPTIONS="-native"
   # needed to be told where to find libm on LinuxMint 14
   LINK_OPTIONS="-native -L/usr/lib/x86_64-linux-gnu"
   LINK_LIBS="-lm"

# default to GCC
else
   COMPILER=gcc
   LINKER=gcc
   
   OPTI="-O3 -ffast-math"
   LINK_OPTIONS=
   LINK_LIBS="-lm"
fi

LANG="-x c -ansi -std=iso9899:199409 -pedantic"
WARN="-Wall -Wextra -Wcast-align -Wwrite-strings -Wpointer-arith -Wredundant-decls -Wdisabled-optimization"
ARCH="-mfpmath=sse -msse"

COMPILE_OPTIONS="-c $LANG $OPTI $ARCH $WARN -Isrc"

EXE_NAME=minilightmerge-c


# compile and link

echo
$COMPILER --version

$COMPILER $COMPILE_OPTIONS ../src/*.c

$LINKER $LINK_OPTIONS -o $EXE_NAME *.o $LINK_LIBS


# move executable and return from build directory

mv $EXE_NAME ..
cd ..
rm obj/*


exit
