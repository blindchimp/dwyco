#!/bin/sh
export CMAKE_CXX_COMPILER_LAUNCHER=ccache

~/Qt/6.8.2/macos/bin/qt-cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_OSX_ARCHITECTURES="arm64;x86_64" -S ~/git/dwyco/phoo -B ~/git/build-phoo

cd ~/git/build-phoo
make -j 8

