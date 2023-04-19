# these are helpers that are used on the client side
#
# ftpreview creates an image thumbnail from a dwyco dyc file
# and is only used on linux servers.
#
# the rest of these things are only used on desktops
#
TEMPLATE = subdirs
CONFIG += ordered

linux {
SUBDIRS=bld/ogg bld/vorbis112 bld/theora.1.2.x bld/speex bld/kazlib bld/zlib bld/dwcls bld/crypto5 bld/jenkins bld/vc bld/pbm bld/pgm bld/ppm bld/gsm bld/cdc32 bld/uv
#SUBDIRS += ftpreview
}

!linux {
SUBDIRS=bld/kazlib bld/zlib bld/dwcls bld/crypto5 bld/jenkins bld/vc bld/pbm bld/pgm bld/ppm bld/gsm bld/cdc32
}

SUBDIRS += dwycobg xferdl
# not built until further notice
SUBDIRS += qdwyrun
