# these are helpers that are used on the client side

TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS=bld/ogg bld/vorbis112 bld/theora bld/speex bld/kazlib bld/zlib bld/dwcls bld/crypto5 bld/jenkins bld/vc bld/pbm bld/pgm bld/ppm bld/gsm bld/cdc32
SUBDIRS += ftpreview dwycobg xferdl
# not built until further notice
#SUBDIRS += qdwyrun
