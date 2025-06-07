#!/bin/sh
# since this is the file we use to deploy, go ahead and
# update the buildtime
(cd ~/git/dwyco/cdcx;./dumptime)

QTVERS=6.8.2
QTFLAVOR=gcc_64
export PATH=~/Qt/$QTVERS/$QTFLAVOR/bin:$PATH
export CMAKE_CXX_COMPILER_LAUNCHER=ccache

~/Qt/$QTVERS/$QTFLAVOR/bin/qt-cmake -D QT_QMAKE_EXECUTABLE="${HOME}/Qt/$QTVERS/$QTFLAVOR/bin/qmake" -D CMAKE_BUILD_TYPE=Release -S ~/git/dwyco/cdcx -B ~/git/build-cdcx -G Ninja

cd ~/git/build-cdcx
ninja

