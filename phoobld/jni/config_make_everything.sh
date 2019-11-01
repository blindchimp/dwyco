#!/bin/bash
. settings.sh
. ndk_autoconf.sh

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

mkdir -p ../obj/local/$NDK_ABI_NAME
cp libtheora/output/lib/*.a libogg/output/lib/*.a libvorbis/output/lib/*.a libspeex/output/lib/*.a ../obj/local/$NDK_ABI_NAME
