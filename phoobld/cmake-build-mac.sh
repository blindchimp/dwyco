#!/bin/sh
QTVERS=6.8.2
export PATH=~/Qt/$QTVERS/macos/bin:$PATH
export CMAKE_CXX_COMPILER_LAUNCHER=ccache

~/Qt/$QTVERS/macos/bin/qt-cmake -D QT_QMAKE_EXECUTABLE="/Users/dwight/Qt/$QTVERS/macos/bin/qmake" -D CMAKE_BUILD_TYPE=Release -D CMAKE_OSX_ARCHITECTURES="arm64;x86_64" -S ~/git/dwyco/phoo -B ~/git/build-phoo -G Ninja
#~/Qt/6.8.2/macos/bin/qt-cmake --build ~/git/build-phoo --target all

cd ~/git/build-phoo
#make -j 8
ninja

