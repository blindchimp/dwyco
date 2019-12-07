#!/bin/bash
. settings.sh

function die {
  echo "$1 failed" && exit 1
}

./configure_ogg.sh || die "OGG configure"
./make_ogg.sh || die "OGG make"

./configure_speex.sh || die "SPEEX configure"
./make_speex.sh || die "SPEEX make"

./configure_vorbis.sh || die "VORBIS configure"
./make_vorbis.sh || die "VORBIS make"

./configure_theora.sh || die "THEORA configure"
./make_theora.sh || die "THEORA make"

if [ "$NDK_ABI" = "arm" ]
then
mkdir -p ../obj/local/armeabi-v7a
cp libtheora/output/lib/*.a libogg/output/lib/*.a libvorbis/output/lib/*.a libspeex/output/lib/*.a ../obj/local/armeabi-v7a/
else
mkdir -p ../obj/local/x86
cp libtheora/output/lib/*.a libogg/output/lib/*.a libvorbis/output/lib/*.a libspeex/output/lib/*.a ../obj/local/x86/
fi
