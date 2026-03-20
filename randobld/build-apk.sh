# this script just builds one debug apk for rando
# this uses the 16kb version of qt 6.8.2 build with ndk 28.2
QTDIR=$HOME/Qt/6.10.2/android_x86_64
export PATH=$QTDIR/bin:$PATH
rm -rf /tmp/r

qt-cmake \
-D QT_QMAKE_EXECUTABLE="$QTDIR/bin/qmake" \
-D ANDROID_SDK_ROOT=$HOME/Android/Sdk \
-D ANDROID_NDK_ROOT=$HOME/Android/Sdk/ndk/28.2.13676358 \
-S $HOME/git/dwyco/rando -B /tmp/r -GNinja 

#-D CMAKE_BUILD_TYPE=Release \
#-D QT_ANDROID_SIGN_APK=ON \
