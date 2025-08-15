cmake_minimum_required(VERSION 3.14)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../${DWYCO_CONFDIR}/conf.cmake)

project(crypto8 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_definitions(NDEBUG CRYTPOPP_DISABLE_ASM)

add_library(crypto8 STATIC

allocate.cpp 
algparam.cpp 
asn.cpp 
authenc.cpp 
base64.cpp 
basecode.cpp 
bfinit.cpp 
blowfish.cpp 
channels.cpp 
cpu.cpp 
cryptlib.cpp 
dh2.cpp 
dh.cpp 
dll.cpp 
files.cpp 
filters.cpp 
fips140.cpp 
gcm.cpp 
gf2n.cpp 
gfpcrypt.cpp 
hex.cpp 
hmac.cpp 
hrtimer.cpp 
integer.cpp 
iterhash.cpp 
md5.cpp 
misc.cpp 
modes.cpp 
mqueue.cpp 
nbtheory.cpp 
oaep.cpp 
osrng.cpp 
pubkey.cpp 
queue.cpp 
randpool.cpp 
rdtables.cpp 
rijndael.cpp 
rng.cpp 
sha.cpp 
sha3.cpp 
strciphr.cpp 
keccak.cpp 
keccak_core.cpp 
ecp.cpp 
ec2n.cpp 
primetab.cpp

)
