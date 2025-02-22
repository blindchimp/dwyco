#!/bin/sh
QTDIR="$HOME/Qt/6.8.2/gcc_64"
export PATH=$QTDIR/bin:$PATH
export CMAKE_CXX_COMPILER_LAUNCHER=ccache

$QTDIR/bin/qt-cmake -D QT_QMAKE_EXECUTABLE="$QTDIR/bin/qmake" -D CMAKE_BUILD_TYPE=Release -S ~/git/dwyco/phoo -B ~/git/build-phoo -G Ninja
#~/Qt/6.8.2/macos/bin/qt-cmake --build ~/git/build-phoo --target all

cd ~/git/build-phoo
#make -j 8
ninja

