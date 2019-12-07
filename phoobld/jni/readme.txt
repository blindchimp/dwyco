dwight melcher
Sat Sep 28 10:49:56 MDT 2019

basic idea for an android build:

get version r20 of the NDK from google

set up the env vars for it, and run the following scripts:
export NDK=~/android/android-ndk (the default)

sh mklinks.sh

run the script

./build-autoconf.sh

this is needed because of the lame autoconf stuff
in the xiph codecs.

# to build the rest of the libs (this will
# automatically build x86,armv7,arm64 libs
./mk.sh

the resulting libs will be in ../libs and ../obj
