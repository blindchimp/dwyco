TEMPLATE = lib
CONFIG += staticlib
CONFIG -= qt

include($$PWD/../../$$DWYCO_CONFDIR/conf.pri)
INCLUDEPATH += \
$${VCCFG_COMP} \
../dwcls \
../zlib \
../kazlib \
../jenkins \
../crypto8 \
../vc \
../pbm \
../pgm \
../ppm \
../gsm \
../speex/include \
../theora.1.2.x/include \
../ogg/include \
../vorbis112/include \
../uv/include \
../miniupnp/miniupnp-master/miniupnpc

linux-*: INCLUDEPATH += ../v4lcap

DEFINES += \
	VCCFG_FILE \
	DWVEC_DOINIT \
        DWYCO_USE_SQLITE \
        DWYCO_NO_VIDEO_FROM_PPM \
        DWYCO_MUTEX_NAME=\\\"$${DWYCO_APP}\\\"
    
equals(DWYCO_APP, "rando") {
DEFINES += DWYCO_NO_THEORA_CODEC DWYCO_NO_GSM DWYCO_NO_VORBIS DWYCO_NO_UPNP DWYCO_NO_VIDEO_FROM_PPM DWYCO_NO_VIDEO_MSGS
#DEFINES += DWYCO_NO_CLEANUP_ON_EXIT
#DEFINES += DWYCO_TRACE
#DEFINES += DWYCO_FIELD_DEBUG
#DEFINES += MINIUPNP_STATICLIB
message("cdc32 setup for rando")
} else {
CONFIG(debug,debug|release) {
#DEFINES += DWYCO_NO_CLEANUP_ON_EXIT
#DEFINES += DWYCO_TRACE
#DEFINES += DWYCO_THREADED_ENCODE
#DEFINES += DW_RTLOG
#DEFINES += DWYCO_FIELD_DEBUG
#android|macx-ios:DEFINES += DWYCO_BACKGROUND_SYNC
DEFINES += DWYCO_SYNC_DEBUG
#DEFINES += DWYCO_DBG_CHECK_SQL
#DEFINES += DWYCO_BACKGROUND_SYNC
message("DEBUG cdc32")
} else {
message("release build")
#android|macx-ios:DEFINES += DWYCO_BACKGROUND_SYNC
}
DEFINES += MINIUPNP_STATICLIB
message("generic setup for cdc32")
}

macx-*|linux-*|macx-ios-clang|macx-clang|android-*|wasm-emscripten {
QMAKE_CXXFLAGS += #-fpermissive
QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-reorder -Wno-unused-variable -Wno-unused-function
INCLUDEPATH += winemu
SOURCES += winemu.cc linid.cpp
}

macx-g++|macx-clang {
DEFINES += UWB_SAMPLING  UWB_SAMPLE_RATE=44100 
DEFINES += MACOSX NEED_SHORT_EXTERNAL_NAMES
QMAKE_CXXFLAGS += -Djpeg_natural_order=dwy_jpeg_natural_order
DEFINES += DWYCO_USE_STATIC_SQLITE
equals(DWYCOBG, 0) {
DEFINES += DWYCO_CDC_LIBUV
}
equals(DWYCOBG, 1) {
DEFINES += DWYCO_NO_THEORA_CODEC DWYCO_NO_GSM DWYCO_NO_VORBIS DWYCO_NO_UPNP DWYCO_NO_VIDEO_FROM_PPM DWYCO_NO_VIDEO_MSGS
}
SOURCES += sqlite3.c
equals(FORCE_DESKTOP_VGQT, 1) {
DEFINES += DWYCO_FORCE_DESKTOP_VGQT
}

# this avoids a bug compiling the old jpeg decoder
#  on macos xcode 16.3 with -O2
# note: removing the inlining from packbits.* seems to
# fix the problem.
#CONFIG(release, debug|release): QMAKE_CXXFLAGS += -O1

}

macx-ios-clang {
DEFINES += MACOSX NEED_SHORT_EXTERNAL_NAMES #DWYCO_THREADED_ENCODE
QMAKE_CXXFLAGS += -Djpeg_natural_order=dwy_jpeg_natural_order
DEFINES += DWYCO_USE_STATIC_SQLITE
SOURCES += sqlite3.c
equals(DWYCOBG, 0) {
DEFINES += DWYCO_CDC_LIBUV
}
}

win32-* {
DEFINES += CDCCORE_STATIC
DEFINES += DWYCO_USE_STATIC_SQLITE
equals(DWYCO_APP, "rando") {
#DEFINES += USE_VFW
#DEFINES += VIDGRAB_HACKS
#DEFINES += UWB_SAMPLING  UWB_SAMPLE_RATE=44100
#SOURCES += uniq.cpp aqaud.cc audwin.cc
SOURCES += uniq.cpp audwin.cc aqaud.cc
} else {
#DEFINES += USE_VFW
#DEFINES += VIDGRAB_HACKS
DEFINES += UWB_SAMPLING  UWB_SAMPLE_RATE=44100
SOURCES += uniq.cpp aqaud.cc audwin.cc
}
SOURCES += sqlite3.c
INCLUDEPATH += ../mtcap
equals(FORCE_DESKTOP_VGQT, 1) {
DEFINES += DWYCO_NO_VIDEO_CAPTURE DWYCO_FORCE_DESKTOP_VGQT
}
#equals(DWYCOBG, 0) {
#DEFINES += DWYCO_CDC_LIBUV
#}
equals(DWYCOBG, 1) {
DEFINES += DWYCO_NO_THEORA_CODEC DWYCO_NO_GSM DWYCO_NO_VORBIS DWYCO_NO_UPNP DWYCO_NO_VIDEO_FROM_PPM DWYCO_NO_VIDEO_MSGS
}
}

android-g++ {
DEFINES += ANDROID LINUX
DEFINES += DWYCO_USE_STATIC_SQLITE
INCLUDEPATH += winemu glob
SOURCES += sqlite3.c glob/glob.c
}

linux-* {
# useful for static qt build
QMAKE_CXXFLAGS += -Djpeg_natural_order=dwy_jpeg_natural_order

DEFINES += UWB_SAMPLING  UWB_SAMPLE_RATE=44100 
DEFINES += DWYCO_USE_STATIC_SQLITE
SOURCES += sqlite3.c
equals(DWYCOBG, 0) {
DEFINES += DWYCO_CDC_LIBUV
}
equals(DWYCOBG, 1) {
DEFINES += DWYCO_NO_THEORA_CODEC DWYCO_NO_GSM DWYCO_NO_VORBIS DWYCO_NO_UPNP DWYCO_NO_VIDEO_FROM_PPM DWYCO_NO_VIDEO_MSGS
#DEFINES += DWYCO_CDC_LIBUV
}
equals(FORCE_DESKTOP_VGQT, 1) {
DEFINES += DWYCO_FORCE_DESKTOP_VGQT
}
}

wasm-emscripten {
DEFINES += UWB_SAMPLING  UWB_SAMPLE_RATE=44100
DEFINES += DWYCO_USE_STATIC_SQLITE
DEFINES += DWYCO_NO_UPNP
QMAKE_CXXFLAGS += -Djpeg_natural_order=dwy_jpeg_natural_order
equals(FORCE_DESKTOP_VGQT, 1) {
DEFINES += DWYCO_FORCE_DESKTOP_VGQT
}
SOURCES += sqlite3.c
}

SOURCES += \
    activeuid.cpp \
    backandroid.cpp \
    bgapp.cpp \
    ezset2.cpp \
mmchan.cc \
mmbld.cc \
mmaud.cc \
mmband.cc \
    mmchan_sync.cpp \
mmconn.cc \
mmctrl.cc \
mmchan2.cc \
mmsync.cc \
audconv.cc \
audchk.cc \
acqfile.cc \
dct.cc \
audmixs.cc \
audout.cc \
bcastq.cc \
chroma.cc \
codec.cc \
colcod.cc \
coldec.cc \
dchroma.cc \
    profiledb.cpp \
    pulls.cpp \
qtab.cc \
dwrate.cc \
dwrtlog.cc \
dwtimer.cc \
enc.cc \
filetube.cc \
gsmconv.cc \
imgmisc.cc \
jccolor.cc \
jchuff.cc \
jcparam.cc \
jdcolor.cc \
jdhuff.cc \
jdmaster.cc \
jfdctfst.cc \
jfdctint.cc \
jqtab.cc \
jtcode.cc \
jtdecode.cc \
jutils.cc \
mo.cc \
netvid.cc \
packbits.cc \
qdirth.cc \
qpol.cc \
dwlog.cc \
    sync_sendq.cpp \
    synccalls.cpp \
syncvar.cc \
doinit.cc \
netcod.cc \
netcod2.cc \
tcode.cc \
statfun.cc \
dirth.cc \
rlc.cc \
tdecode.cc \
tpgmdec.cc \
sqrs.cc \
qauth.cc \
senc.cc \
aq.cc \
vcpan.cc \
ta.cc \
audi.cc \
audo.cc \
aconn.cc \
tpgmcod.cc \
globs.cc \
fl.cc \
mmserv.cc \
tl.cc \
dlli.cpp \
mcc.cpp \
qmsg.cc \
audexto.cc \
aqext.cc \
aqextaud.cc \
audth.cc \
audmix.cc \
autoup.cpp \
dmdsrv.cc \
fnmod.cc \
xinfo.cpp \
vidcvt.cc \
callq.cpp \
asshole.cpp \
servass.cpp \
chatops.cc \
mmcall.cpp \
calllive.cpp \
spxconv.cc \
calldll.cpp \
msgddll.cpp \
jcolcod.cc \
jcoldec.cc \
netdiag.cpp \
pgdll.cpp \
chatgrid.cpp \
chatq.cpp \
sysattr.cpp \
vorbconv.cc \
se.cpp \
theoracol.cc \
ser.cpp \
mmchan3.cpp \
sproto.cpp \
dhsetup.cpp \
trc.cpp \
cdcpal2.cc \
ssns.cpp \
qsend.cpp \
directsend.cpp \
msend.cpp \
qmsgsql.cpp \
fetch_to_inbox.cpp \
sqlbq.cpp \
aqext_android.cpp \
    dhgsetup.cpp \
    simplesql.cpp \
    grpmsg.cpp \
    upnp.cpp \
    aqkey.cpp \
    netlog.cpp

HEADERS += \
    backandroid.h





