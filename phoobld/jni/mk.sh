#!/bin/bash
. settings.sh 

if [ ! -d "$NDK" ]
then
if [ "$(uname)" = "Darwin" ]; then
    export NDK=~/Library/Android/sdk/ndk/21.3.6528147
else
    export NDK=~/Android/Sdk/ndk/21.3.6528147
fi
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
$NDK/ndk-build   -j12 #NDK_DEBUG=1
