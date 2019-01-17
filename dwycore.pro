TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS= \
bld/ogg \
bld/vorbis112 \
bld/theora \
bld/speex \
bld/kazlib \
bld/zlib \
bld/dwcls \
bld/crypto5 \
bld/jenkins \
bld/vc \
bld/pbm \
bld/pgm \
bld/ppm \
bld/gsm \
bld/cdc32 \
bld/miniupnp/miniupnp-master/miniupnpc

linux-*:SUBDIRS += bld/v4lcap

SUBDIRS += cdcx
