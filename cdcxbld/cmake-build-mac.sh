#!/bin/sh
# since this is the file we use to deploy, go ahead and
# update the buildtime
(cd ~/git/dwyco/cdcx;./dumptime-mac)

QTVERS=6.8.2
export PATH=~/Qt/$QTVERS/macos/bin:$PATH
export CMAKE_CXX_COMPILER_LAUNCHER=ccache

~/Qt/$QTVERS/macos/bin/qt-cmake -D QT_QMAKE_EXECUTABLE="/Users/dwight/Qt/$QTVERS/macos/bin/qmake" -D CMAKE_BUILD_TYPE=Release -D CMAKE_OSX_ARCHITECTURES="arm64;x86_64" -S ~/git/dwyco/cdcx -B ~/git/build-cdcx -G Ninja

cd ~/git/build-cdcx
ninja

