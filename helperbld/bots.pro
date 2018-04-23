# these are bots that are used on the server side (though they act like
# clients, so they can be run almost anywhere)

TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS=bld/ogg bld/vorbis112 bld/theora bld/speex bld/kazlib bld/zlib bld/dwcls bld/crypto5 bld/jenkins bld/vc bld/pbm bld/pgm bld/ppm bld/gsm bld/cdc32
SUBDIRS += picbot greetbot
