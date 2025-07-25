TEMPLATE = lib
CONFIG += staticlib
CONFIG -= qt
CONFIG += warn_off
macx-g++|macx-clang|linux-g++|macx-ios-* {
# this monkey business forces the use of c++11 when
# compiling with newer versions of qt, which puts
# flags on the command line for newer c++ versions, which
# breaks compilation of this library.
QMAKE_CXXFLAGS -= -std=c++1z
QMAKE_CXXFLAGS -= -std=gnu++1z
QMAKE_CXXFLAGS_CXX1Z=""
QMAKE_CXXFLAGS_GNUCXX1Z=""
QMAKE_CXXFLAGS += -Wno-unused-variable -Wno-unused-parameter $$QMAKE_CXXFLAGS_CXX11
}

windows {
# get rid of c++17, this lib doesn't need it
QMAKE_CXXFLAGS_CXX1Z=""
}

include($$PWD/../../$$DWYCO_CONFDIR/conf.pri)
DEFINES += NDEBUG 

SOURCES = \
algparam.cpp \
asn.cpp \
authenc.cpp \
base64.cpp \
basecode.cpp \
bfinit.cpp \
blowfish.cpp \
channels.cpp \
cpu.cpp \
cryptlib_bds.cpp \
cryptlib.cpp \
dh2.cpp \
dh.cpp \
dll.cpp \
files.cpp \
filters.cpp \
fips140.cpp \
gcm.cpp \
gf2n.cpp \
gfpcrypt.cpp \
hex.cpp \
hmac.cpp \
hrtimer.cpp \
integer.cpp \
iterhash.cpp \
md5.cpp \
misc.cpp \
modes.cpp \
mqueue.cpp \
nbtheory.cpp \
oaep.cpp \
osrng.cpp \
pubkey.cpp \
queue.cpp \
randpool.cpp \
rdtables.cpp \
rijndael.cpp \
rng.cpp \
sha.cpp \
sha3.cpp

#\
#   des.cpp \
#    dessp.cpp

#ripemd.cpp \
#wait.cpp \
#winpipes.cpp \
#xtr.cpp \
#xtrcrypt.cpp \
#socketft.cpp \
#square.cpp \
#squaretb.cpp \
#strciphr.cpp \
#
#3way.cpp \
#adler32.cpp \
#algebra.cpp \
#algparam.cpp \
#arc4.cpp \
#asn.cpp \
#authenc.cpp \
#base32.cpp \
#base64.cpp \
#basecode.cpp \
#bfinit.cpp \
#blowfish.cpp \
#blumshub.cpp \
#camellia.cpp \
#cast.cpp \
#casts.cpp \
#cbcmac.cpp \
#ccm.cpp \
#channels.cpp \
#cmac.cpp \
#cpu.cpp \
#crc.cpp \
#cryptlib.cpp \
#default.cpp \
#des.cpp \
#dessp.cpp \
#dh2.cpp \
#dh.cpp \
#dll.cpp \
#dsa.cpp \
#eax.cpp \
#ec2n.cpp \
#eccrypto.cpp \
#ecp.cpp \
#elgamal.cpp \
#emsa2.cpp \
#eprecomp.cpp \
#esign.cpp \
#files.cpp \
#filters.cpp \
#fips140.cpp \
#gcm.cpp \
#gf2_32.cpp \
#gf256.cpp \
#gf2n.cpp \
#gfpcrypt.cpp \
#gost.cpp \
#gzip.cpp \
#hex.cpp \
#hmac.cpp \
#hrtimer.cpp \
#ida.cpp \
#idea.cpp \
#integer.cpp \
#iterhash.cpp \
#luc.cpp \
#mars.cpp \
#marss.cpp \
#md2.cpp \
#md4.cpp \
#md5.cpp \
#misc.cpp \
#modes.cpp \
#mqueue.cpp \
#mqv.cpp \
#nbtheory.cpp \
#network.cpp \
#oaep.cpp \
#osrng.cpp \
#panama.cpp \
#pch.cpp \
#pkcspad.cpp \
#polynomi.cpp \
#pssr.cpp \
#pubkey.cpp \
#queue.cpp \
#rabin.cpp \
#randpool.cpp \
#rc2.cpp \
#rc5.cpp \
#rc6.cpp \
#rdtables.cpp \
#rijndael.cpp \
#ripemd.cpp \
#rng.cpp \
#rsa.cpp \
#rw.cpp \
#safer.cpp \
#salsa.cpp \
#seal.cpp \
#seed.cpp \
#serpent.cpp \
#shacal2.cpp \
#sha.cpp \
#sharkbox.cpp \
#shark.cpp \
#simple.cpp \
#skipjack.cpp \
#socketft.cpp \
#sosemanuk.cpp \
#square.cpp \
#squaretb.cpp \
#strciphr.cpp \
#tea.cpp \
#tftables.cpp \
#tiger.cpp \
#tigertab.cpp \
#trdlocal.cpp \
#ttmac.cpp \
#twofish.cpp \
#vmac.cpp \
#wait.cpp \
#wake.cpp \
#whrlpool.cpp \
#winpipes.cpp \
#xtr.cpp \
#xtrcrypt.cpp \
#zdeflate.cpp \
#zinflate.cpp \
#zlib.cpp 

