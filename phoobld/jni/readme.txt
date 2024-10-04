Sat Sep 28 10:49:56 MDT 2019

on Macos, using homebrew, be sure you have autoconf installed:

brew install autoconf automake autotools libtool pkg-config
(note: some of them may not exist depending on macos version you are using)

basic idea for an android build:

get the version of the NDK required for the version of Qt you are building for:
Qt 6.5.3 NDK 25.1.8937393
Qt 6.7.3 NDK 26.1.10909125

export NDK=~/android/android-ndk (the default)

(NOTE: if it can't find the NDK there, there are defaults in settings.sh
that assume you have installed the android SDK and it has downloaded
the NDK's for you. ca 2024, QtCreator mercifully does most of this for you.)

WARNING WARNING WARNING:
on a fresh install, be sure to run "distclean.sh" to get rid of all the old cruft, otherwise, autoconf may fail with really obscure and useless error messages.
NOTE! also the "distclean" can cause autoconf to go into an infinite loop. jesus, what a POS.

sh mklinks.sh
./distclean.sh

./build-autoconf.sh

this is needed because of the lame autoconf stuff
in the xiph codecs.

# to build the rest of the libs (this will
# automatically build x86,x86_64,armv7,arm64 libs
./mk.sh

the resulting libdwyco.so will be in ../libs and ../obj
NOTE! there is no dependency on Qt directly for this library. the primary reason you need different versions of the NDK base on the Qt version is just to keep things consistent. You *might* be able to link this lib with whatever version of the NDK you want, separate from Qt, but I haven't tried that.
