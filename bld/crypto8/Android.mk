LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := crypto8

LOCAL_CPPFLAGS := $(MY_CONF_LOCAL_CPPFLAGS) -DCRYPTOPP_DISABLE_ASM -DNDEBUG -DIS_LITTLE_ENDIAN -DANDROID -frtti -fexceptions

LOCAL_SRC_FILES := \
allocate.cpp \
algparam.cpp \
asn.cpp \
authenc.cpp \
base64.cpp \
basecode.cpp \
bfinit.cpp \
blowfish.cpp \
channels.cpp \
cpu.cpp \
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
sha3.cpp \
strciphr.cpp \
keccak.cpp \
keccak_core.cpp \
ecp.cpp \
ec2n.cpp \
primetab.cpp

include $(BUILD_STATIC_LIBRARY)

