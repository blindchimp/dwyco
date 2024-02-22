#!/bin/bash

# set to path of your NDK (or export NDK to environment)

if [[ "x$NDK" == "x" ]]; then
#NDK=~/android/android-ndk
	if [ `uname` = "Darwin" ]
	then
	#NDK=~/Library/Android/sdk/ndk/21.3.6528147/
	NDK=~/Library/Android/sdk/ndk/25.1.8937393/
	else
	#NDK=~/Android/Sdk/ndk/21.3.6528147
	NDK=~/Android/Sdk/ndk/25.1.8937393
	fi
fi
# i use only a small number of formats - set this to 0 if you want everything.
# changed 0 to the default, so it'll compile shitloads of codecs normally
if [[ "x$minimal_featureset" == "x" ]]; then
minimal_featureset=0
fi

if [ -z "$NDK_ABI" ]
then
	echo NDK_ABI should be \"arm\", \"x86\", \"arm64\", \"x86_64\"
	if [ -f NDK_ABI_X86 ]
	then
		echo setting NDK_ABI to \"x86\"
		export NDK_ABI="x86"
		export TARGET_TAG="i686-linux-android"
		export NDK_ABI_NAME="x86"
	elif [ -f NDK_ABI_X86_64 ]
	then
		echo setting NDK_ABI to \"x86_64\"
		export NDK_ABI="x86_64"
		export TARGET_TAG="x86_64-linux-android"
		export NDK_ABI_NAME="x86_64"
	elif [ -f NDK_ABI_ARM64 ]
	then
		echo setting NDK_ABI to \"arm64\"
		export NDK_ABI="arm64"
		export TARGET_TAG="aarch64-linux-android"
		export NDK_ABI_NAME="arm64-v8a"
	else
		echo setting NDK_ABI to \"arm\"
		export NDK_ABI="arm"
		export TARGET_TAG="armv7a-linux-androideabi"
		export NDK_ABI_NAME="armeabi-v7a"
	fi
fi

## stop editing

if [[ ! -d $NDK ]]; then
  echo "$NDK is not a directory. Exiting."
  exit 1
fi

function current_dir {
  echo "$(cd "$(dirname $0)"; pwd)"
}

export PATH=$NDK:$PATH

echo $PATH

