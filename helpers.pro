# these are helpers that are used on the client side
#
# these are only used on desktops
#
TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS=bld/kazlib bld/zlib bld/dwcls bld/crypto8 bld/jenkins bld/vc bld/pgm bld/ppm bld/cdc32
linux {
SUBDIRS += bld/uv
}

# distributed on desktop platforms for background processing.
# does not depend on Qt
SUBDIRS += dwycobg
# background update downloader. hasn't been used in ages, but still
# distributed on windows. no qt dependencies.
SUBDIRS += xferdl
# builds, but only used on qt6/windows
SUBDIRS += qdwyrun
