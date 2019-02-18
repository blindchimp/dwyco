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

LOCAL_CPPFLAGS += #-DDW_RTLOG -DDWYCO_NO_CLEANUP_ON_EXIT -DDWYCO_APP_DEBUG
#LOCAL_CPPFLAGS += -DDWYCO_TRACE 
#LOCAL_CPPFLAGS += -DLEAK_CLEANUP
#LOCAL_CPPFLAGS += -DDWYCO_FIELD_DEBUG
#LOCAL_CPPFLAGS += -DDWYCO_THREADED_ENCODE
LOCAL_CPPFLAGS += -DDWYCO_NO_THEORA_CODEC -DDWYCO_NO_GSM -DDWYCO_NO_VORBIS -DDWYCO_NO_UPNP

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
cllaccpt.cpp \
ratetwkr.cpp \
rawfiles.cpp \
syncvar.cc \
uicfg.cc \
usercnfg.cpp \
vfwinvst.cpp \
vidinput.cpp \
zapadv.cpp \
doinit.cc \
netcod.cc \
tcode.cc \
sleep.cc \
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
cdcpal.cc \
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
prfcache.cpp \
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
ezset.cpp \
linid.cpp \
se.cpp \
theoracol.cc \
ser.cpp  \
dhsetup.cpp \
sproto.cpp \
mmchan3.cpp \
trc.cpp \
glob/glob.c \
pkcache.cpp \
ssns.cpp \
qsend.cpp \
directsend.cpp \
msend.cpp \
qmsgsql.cpp \
fetch_to_inbox.cpp \
sqlbq.cpp \
favmsg.cpp \
sqlite3.c \
dwybg_wrap.c \
aqext_android.cpp \
backsql.cpp \
upnp.cpp

LOCAL_STATIC_LIBRARIES := ppm pgm pbm vc crypto5 zlib kazlib jenkins dwcls
LOCAL_C_INCLUDES := \
$(LOCAL_PATH)/../dwcls \
$(LOCAL_PATH)/../zlib \
$(LOCAL_PATH)/../kazlib \
$(LOCAL_PATH)/../jenkins \
$(LOCAL_PATH)/../crypto5 \
$(LOCAL_PATH)/../vc \
$(LOCAL_PATH)/../lpc \
$(LOCAL_PATH)/../pbm \
$(LOCAL_PATH)/../pgm \
$(LOCAL_PATH)/../ppm \
$(LOCAL_PATH)/winemu \
$(LOCAL_PATH)/glob \
$(LOCAL_PATH)/../../randobld

LOCAL_CPPFLAGS += -fpermissive -frtti


#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_STATIC_LIBRARY)
