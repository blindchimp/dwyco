dwight melcher
Mon Apr 15 13:47:20 MST 2013

basic idea for an android build:
get version r13 of the NDK from google

set up the env vars for it, and run the following scripts:
export NDK=~/android/android-ndk (the default)

the scripts here expect this:

export NDK_ABI= "arm" or "x86" (arm is the default)

sh mklinks.sh
./create_toolchain.sh

# to build external things like vorbis
./config_make_everything.sh


# to build the rest of the lib
./mk.sh

the resulting libs will be in ../libs and ../obj
