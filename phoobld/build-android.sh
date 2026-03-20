
# note: it isn't clear from the docs that we spec the signing here while configuring, but
# the details of the signing keys and whatnot are provided via environment
# variables during the build process. maybe there is a way to do that
# in cmake i'm just not aware of...
#
# so, to use this script, you run this first
# then cd /tmp/r
# and run the build-aab.sh script that is in
# deploy-rando.
# we don't put build-aab.sh here since it contains private keys.
#
# note: we don't need to set a bunch of stuff since qt-cmake
# seems to bind itself by default to the "host build" when we
# built android-qt

#/home/dwight/android682_arm64-v8a/bin/qt-cmake -S /home/dwight/git/dwyco/rando -B /tmp/r -GNinja
#-D QT_QMAKE_EXECUTABLE="$QTDIR/bin/qmake" 
QTDIR=$HOME/Qt/6.10.2/android_x86_64
export PATH=$QTDIR/bin:$PATH
rm -rf /tmp/r

qt-cmake \
-D QT_QMAKE_EXECUTABLE="$QTDIR/bin/qmake" \
-D CMAKE_BUILD_TYPE=Release \
-S $HOME/git/dwyco/phoo -B /tmp/r -GNinja \
-D QT_NO_GLOBAL_APK_TARGET_PART_OF_ALL:BOOL=OFF \
-D ANDROID_SDK_ROOT=$HOME/Android/Sdk \
-D ANDROID_NDK_ROOT=$HOME/Android/Sdk/ndk/28.2.13676358 \
-D QT_ANDROID_BUILD_ALL_ABIS=TRUE \
-D QT_ANDROID_SIGN_AAB=TRUE 

#-DQT_PATH_ANDROID_ABI_arm64-v8a="$HOME/syncdev/qta6101/android6101_arm64-v8a" \
#-DQT_PATH_ANDROID_ABI_x86_64="$HOME/syncdev/qta6101/android6101_x86_64" \
#-DQT_PATH_ANDROID_ABI_armeabi-v7a="$HOME/syncdev/qta6101/android6101_armeabi-v7a" \
#-DQT_PATH_ANDROID_ABI_x86="$HOME/syncdev/qta6101/android6101_x86" \
