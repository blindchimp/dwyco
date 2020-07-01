#!/bin/sh
SHADOW_NAME=$1

mkdir $SHADOW_NAME
mkdir $SHADOW_NAME/lib
mkdir $SHADOW_NAME/include

# note, this was hijacked to do this while doing
# libuv too. probably need to fix this as well

if [ ! -e $SHADOW_NAME/include/sp.h ]
then
( 
cd bld/spread-hacked 
cd `cat curver` 
./configure --prefix=$SHADOW_NAME 
make -j 8 install 
) 
fi
