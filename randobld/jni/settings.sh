#!/bin/bash

# set to path of your NDK (or export NDK to environment)

if [[ "x$NDK" == "x" ]]; then
NDK=~/android/android-ndk
fi
# i use only a small number of formats - set this to 0 if you want everything.
# changed 0 to the default, so it'll compile shitloads of codecs normally
if [[ "x$minimal_featureset" == "x" ]]; then
minimal_featureset=0
fi

if [ -z "$NDK_ABI" ]
then
	echo NDK_ABI should be either \"arm\" or \"x86\"
	if [ -f NDK_ABI_X86 ]
	then
		echo setting NDK_ABI to \"x86\"
		export NDK_ABI="x86"
	else
		echo setting NDK_ABI to \"arm\"
		export NDK_ABI="arm"
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

export PATH=$NDK:$(current_dir)/toolchain/bin:$PATH

echo $PATH

