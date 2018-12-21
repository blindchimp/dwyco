#!/bin/bash
. settings.sh 

if [ ! -d "$NDK" ]
then
export NDK=~/android/android-ndk
fi
echo $NDK

export NDK_CCACHE=ccache
if [ "$NDK_ABI" = "arm" ]
then
export NDK_PROJECT_PATH=~/git/dwyco/phoobld
else
export NDK_PROJECT_PATH=~/git/dwyco/phoobld
fi
(cd ..;mkdir obj)
# force rebuild all
#$NDK/ndk-build -B NDK_DEBUG=1
$NDK/ndk-build  #NDK_DEBUG=1
