# this script just builds one debug apk for rando
# this uses the 16kb version of qt 6.8.2 build with ndk 28.2
QTDIR=$HOME/qt6101
rm -rf /tmp/r

$HOME/syncdev/qta6101/android6101_arm64-v8a/bin/qt-cmake \
-D QT_QMAKE_EXECUTABLE="$QTDIR/bin/qmake" \
-D CMAKE_BUILD_TYPE=Release \
-D QT_ANDROID_SIGN_APK=ON \
-S $HOME/git/dwyco/rando -B /tmp/r -GNinja 
