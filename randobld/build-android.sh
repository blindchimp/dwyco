
# note: it isn't clear from the docs that we spec the signing here whle configuring, but
# the details of the signing keys and whatnot are provided via environment
# variables during the build process. maybe there is a way to do that
# in cmake i'm just not aware of...
#
# so, to use this script, you run this first
# then cd /tmp/r
# and run the build-aab.sh script that is in
# deploy-rando.
# we don't put build-aab.sh here since it contains private keys.

#/home/dwight/android682_arm64-v8a/bin/qt-cmake -S /home/dwight/git/dwyco/rando -B /tmp/r -GNinja
/home/dwight/syncdev/qta682/android682_x86_64/bin/qt-cmake -S /home/dwight/git/dwyco/rando -B /tmp/r -GNinja \
-DQT_ANDROID_BUILD_ALL_ABIS=TRUE \
-DQT_PATH_ANDROID_ABI_arm64-v8a="/home/dwight/syncdev/qta682/android682_arm64_v8a" \
-DQT_PATH_ANDROID_ABI_x86_64="/home/dwight/syncdev/qta682/android682_x86_64" \
-DQT_PATH_ANDROID_ABI_armeabi-v7a="/home/dwight/syncdev/qta682/android682_armv7" \
-DQT_PATH_ANDROID_ABI_x86="/home/dwight/syncdev/qta682/android682_x86" \
-DQT_ANDROID_SIGN_AAB=TRUE 
