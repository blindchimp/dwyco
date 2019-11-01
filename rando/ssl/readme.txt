Tue Mar 28 07:07:03 MST 2017
this is a hack to build openssl for distribution on android, since
Qt seems to probe for ssl even though the app doesn't use it, resulting
in errors being spewed out, and apps crashing. this is a bug in qt, but
it isn't going to be fixed...

so build openssl and strip the resulting .so files, and put them into
the app... this works, and seems to quell most of the problems.
note: this script does *not* work with newer versions of openssl, since
this is a hack, i'm not going to worry about it.
NOTE: NDK needs to be ndk-r13
NOTE: for whatever reason, scripts fail on x86, probably all the goofy flags sent to compiler to support all the stupid asm code

./d.sh $NDK ~/Downloads/openssl-1.0.2k 19 arm 4.9 ~/Downloads/arm
./d.sh $NDK ~/Downloads/openssl-1.0.2k 19 x86 4.9 ~/Downloads/x86

