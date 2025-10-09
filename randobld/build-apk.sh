# this script just builds one debug apk for rando
# this uses the 16kb version of qt 6.8.2 build with ndk 28.2

$HOME/syncdev/qta682/android682_arm64_v8a/bin/qt-cmake -S $HOME/git/dwyco/rando -B /tmp/r -GNinja 
