cmake_minimum_required(VERSION 3.14)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../${DWYCO_CONFDIR}/conf.cmake)

project(crypto5 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_definitions(NDEBUG)

add_library(crypto5 STATIC
#algparam.cpp 
#asn.cpp 
#authenc.cpp 
#base64.cpp 
#basecode.cpp 
#bfinit.cpp 
#blowfish.cpp 
#channels.cpp 
#cpu.cpp 
#cryptlib_bds.cpp 
#cryptlib.cpp 
#dh2.cpp 
#dh.cpp 
#dll.cpp 
#files.cpp 
#filters.cpp 
#fips140.cpp 
#gcm.cpp 
#gf2n.cpp 
#gfpcrypt.cpp 
#hex.cpp 
#hmac.cpp 
#hrtimer.cpp 
#integer.cpp 
#iterhash.cpp 
#md5.cpp 
#misc.cpp 
#modes.cpp 
#mqueue.cpp 
#nbtheory.cpp 
#oaep.cpp 
#osrng.cpp 
#pubkey.cpp 
#queue.cpp 
#randpool.cpp 
#rdtables.cpp 
#rijndael.cpp 
#rng.cpp 
#sha.cpp 
#sha3.cpp

rdtables.cpp
osrng.cpp
hrtimer.cpp
sha.cpp
cryptlib_bds.cpp
fips140.cpp
cpu.cpp
bfinit.cpp
misc.cpp
algparam.cpp
oaep.cpp
nbtheory.cpp
hmac.cpp
mqueue.cpp
pubkey.cpp
modes.cpp
iterhash.cpp
integer.cpp
gfpcrypt.cpp
dll.cpp
queue.cpp
filters.cpp
cryptlib.cpp
asn.cpp
sha3.cpp
rng.cpp
rijndael.cpp
md5.cpp
randpool.cpp
gf2n.cpp
files.cpp
dh2.cpp
dh.cpp
channels.cpp
blowfish.cpp
basecode.cpp
base64.cpp
authenc.cpp
hex.cpp
gcm.cpp
)
