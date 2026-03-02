#!/bin/sh

rm -rf /tmp/b

export NDK=$HOME/Android/Sdk/ndk/28.2.13676358

cmake -S . -B /tmp/b \
	-DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake \
	-DANDROID_ABI=x86_64 -DANDROID_PLATFORM=android-26 \
	-DANDROID_STL=c++_shared \
	-DCMAKE_BUILD_TYPE=Release -G Ninja
