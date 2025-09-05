# helpers on the server. note: may require private key files to compile
# that are not included in the distribution

TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS=bld/kazlib bld/dwcls bld/crypto8 bld/jenkins bld/vc
SUBDIRS += xfer
