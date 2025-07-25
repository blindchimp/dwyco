LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := cdc32

LOCAL_CPPFLAGS := -x c++ $(MY_CONF_LOCAL_CPPFLAGS) -DANDROID \
	-DVCCFG_FILE \
	-DDWVEC_DOINIT \
	-DDWYCO_USE_STATIC_SQLITE \
	-DUWB_SAMPLING \
	-DUWB_SAMPLE_RATE=8000 \

# only thing that needs these flags is local static sqlite3.c
LOCAL_CFLAGS := $(MY_CONF_LOCAL_CFLAGS) \
	-DDWYCO_USE_STATIC_SQLITE

LOCAL_CPP_EXTENSION := .cc .cpp

#LOCAL_CPPFLAGS += -DDW_RTLOG #-DDWYCO_NO_CLEANUP_ON_EXIT -DDWYCO_APP_DEBUG
#LOCAL_CPPFLAGS += -DDWYCO_NETLOG
#LOCAL_CPPFLAGS += -DDW_ANDROID_LOG
#LOCAL_CPPFLAGS += -DDWYCO_DO_USER_LOG
#LOCAL_CPPFLAGS += -DDWYCO_CDC_LIBUV
#LOCAL_CPPFLAGS += -DDWYCO_TRACE 
#LOCAL_CPPFLAGS += -DLEAK_CLEANUP
#LOCAL_CPPFLAGS += -DDWYCO_FIELD_DEBUG
ifeq ($(DWYCO_APP), "rando")
#LOCAL_CPPFLAGS += -DDWYCO_THREADED_ENCODE
LOCAL_CPPFLAGS += -DDWYCO_NO_THEORA_CODEC -DDWYCO_NO_GSM -DDWYCO_NO_VORBIS -DDWYCO_NO_UPNP -DDWYCO_NO_VIDEO_MSGS -DDWYCO_NO_VIDEO_FROM_PPM
else
LOCAL_CPPFLAGS += -DDWYCO_THREADED_ENCODE
#LOCAL_CPPFLAGS += -DDWYCO_BACKGROUND_SYNC
endif

LOCAL_SRC_FILES=  \
mmchan.cc \
mmbld.cc \
mmaud.cc \
mmband.cc \
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
rawconv.cc \
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
cdcpal2.cc \
dlli.cpp \
mcc.cpp \
qmsg.cc \
winemu.cc \
audexto.cc \
aqext.cc \
aqextaud.cc \
audth.cc \
audmix.cc \
autoup.cpp \
dmdsrv.cc \
fnmod.cc \
xinfo.cpp \
profiledb.cpp \
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
ezset2.cpp \
linid.cpp \
se.cpp \
theoracol.cc \
ser.cpp  \
dhsetup.cpp \
sproto.cpp \
mmchan3.cpp \
trc.cpp \
glob/glob.c \
ssns.cpp \
qsend.cpp \
directsend.cpp \
msend.cpp \
qmsgsql.cpp \
fetch_to_inbox.cpp \
sqlbq.cpp \
sqlite3.c \
dwybg_wrap.c \
aqext_android.cpp \
upnp.cpp \
simplesql.cpp \
aqkey.cpp \
pulls.cpp \
dhgsetup.cpp \
grpmsg.cpp \
mmchan_sync.cpp \
sync_sendq.cpp \
bgapp.cpp \
synccalls.cpp \
netlog.cpp \
activeuid.cpp \
backandroid.cpp

ifeq ($(DWYCO_APP), "rando")
LOCAL_STATIC_LIBRARIES := pbm vc crypto8 kazlib jenkins dwcls
else
LOCAL_STATIC_LIBRARIES := libspeexdsp-prebuilt libvorbis-prebuilt libvorbisenc-prebuilt libvorbisfile-prebuilt libtheora-prebuilt libtheoraenc-prebuilt libtheoradec-prebuilt libogg-prebuilt gsm  ppm pgm pbm vc crypto8 kazlib jenkins dwcls miniupnpc
endif

LOCAL_C_INCLUDES := \
$(LOCAL_PATH)/../dwcls \
$(LOCAL_PATH)/../kazlib \
$(LOCAL_PATH)/../jenkins \
$(LOCAL_PATH)/../crypto8 \
$(LOCAL_PATH)/../vc \
$(LOCAL_PATH)/../lpc \
$(LOCAL_PATH)/../pbm \
$(LOCAL_PATH)/../pgm \
$(LOCAL_PATH)/../ppm \
$(LOCAL_PATH)/winemu \
$(LOCAL_PATH)/glob \
$(LOCAL_PATH)/../gsm \
$(LOCAL_PATH)/../speex/include \
$(LOCAL_PATH)/../theora/include \
$(LOCAL_PATH)/../ogg/include \
$(LOCAL_PATH)/../vorbis/include \
$(LOCAL_PATH)/../miniupnp/miniupnp-master/miniupnpc \
$(LOCAL_PATH)/../uv/include \
$(APP_PROJECT_PATH)

LOCAL_CPPFLAGS += -fpermissive -frtti


#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_STATIC_LIBRARY)
