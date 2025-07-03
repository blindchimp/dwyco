TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS= \
bld/ogg \
bld/vorbis112 \
bld/theora.1.2.x \
bld/speex \
bld/kazlib \
bld/dwcls \
bld/crypto8 \
bld/jenkins \
bld/vc \
bld/pbm \
bld/pgm \
bld/ppm \
bld/gsm \
bld/cdc32 \
bld/miniupnp/miniupnp-master/miniupnpc \
bld/uv

linux-*:SUBDIRS += bld/qt6drv bld/v4lcap
macx-*:SUBDIRS += bld/qt6drv
win32-*:SUBDIRS += bld/qt6drv

SUBDIRS += cdcx
