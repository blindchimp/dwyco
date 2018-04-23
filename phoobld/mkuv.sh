#!/bin/sh
SHADOW_NAME=$1

mkdir $SHADOW_NAME
mkdir $SHADOW_NAME/lib
mkdir $SHADOW_NAME/include

(cd bld/libuv
make -j 8
cp libuv.a $SHADOW_NAME/lib
cp include/uv.h $SHADOW_NAME/include
)

