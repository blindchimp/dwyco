Wed Oct  2 08:50:27 AM MDT 2024

basic idea for an android build:

get the version of the NDK required for the version of Qt you are building for:
Qt 6.5.3 NDK 25.1.8937393
Qt 6.7.3 NDK 26.1.10909125

export NDK=~/android/android-ndk (the default)

(NOTE: if it can't find the NDK there, there are defaults in settings.sh
that assume you have installed the android SDK and it has downloaded
the NDK's for you. ca 2024, QtCreator mercifully does most of this for you.)

thankfully, rando does not depend on any of the old xiph codecs that use autoconf to build. you'll need ccache.

# create some symlinks
sh mklinks.sh

# to build the rest of the lib
./mk.sh

the resulting libs will be in ../libs and ../obj
