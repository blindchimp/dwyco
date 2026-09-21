# these are bots that are used on the server side (though they act like
# clients, so they can be run almost anywhere)

TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS=bld/kazlib bld/zlib bld/dwcls bld/crypto8 bld/jenkins bld/vc bld/pgm bld/ppm bld/gsm bld/cdc32
SUBDIRS += randobot picbot greetbot clbot nobot
