#!/bin/bash
. settings.sh 

if [ ! -d "$NDK" ]
then
export NDK=~/Android/Sdk/ndk/21.3.6528147
fi
echo $NDK

export NDK_CCACHE=ccache
if [ "$NDK_ABI" = "arm" ]
then
export NDK_PROJECT_PATH=~/git/dwyco/randobld
else
export NDK_PROJECT_PATH=~/git/dwyco/randobld
fi
(cd ..;mkdir obj)
# force rebuild all
#$NDK/ndk-build -B NDK_DEBUG=1
$NDK/ndk-build   -B -j8 #NDK_DEBUG=1
