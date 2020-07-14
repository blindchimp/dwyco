dwight melcher
Mon Apr 15 13:47:20 MST 2013

basic idea for an android build:
get latest NDK from google, r21 seems to be ok

set up the env vars for it, and run the following scripts:
export NDK=~/android/android-ndk (the default)

the scripts here expect this:

sh mklinks.sh

# to build the rest of the lib
./mk.sh

the resulting libs will be in ../libs and ../obj
