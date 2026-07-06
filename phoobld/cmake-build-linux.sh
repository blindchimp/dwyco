#!/bin/sh
QTDIR="$HOME/Qt/6.11.1/gcc_64"
export PATH=$QTDIR/bin:$PATH
export CMAKE_CXX_COMPILER_LAUNCHER=ccache

$QTDIR/bin/qt-cmake -D QT_QMAKE_EXECUTABLE="$QTDIR/bin/qmake" -D CMAKE_BUILD_TYPE=Release -D DWYCO_TOXCORE=ON -S ~/git/dwyco/phoo -B ~/git/build-phoo -G Ninja
#~/Qt/6.11.1/macos/bin/qt-cmake --build ~/git/build-phoo --target all

cd ~/git/build-phoo
#make -j 8
ninja

