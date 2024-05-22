LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := vc

LOCAL_CPPFLAGS := $(MY_CONF_LOCAL_CPPFLAGS) -DANDROID -DVC_INTERNAL -DDWYCO_NO_TSOCK

LOCAL_SRC_FILES=  \
vclhnet.cpp \
dwgrows.cpp \
pmatch.cpp \
vc.cpp \
vc2.cpp \
vc3.cpp \
vcatomic.cpp \
vccfun.cpp \
vccomp.cpp \
vcctx.cpp \
vccvar.cpp \
vcdbg.cpp \
vcdbl.cpp \
vcdecom.cpp \
vcdeflt.cpp \
vcenco.cpp \
vcexcctx.cpp \
vcfunc.cpp \
vcfuncal.cpp \
vcfunctx.cpp \
vcfundef.cpp \
vcint.cpp \
vclex.cpp \
vclh.cpp \
vcmap.cpp \
vcnil.cpp \
vcregex.cpp \
vcser.cpp \
vcsock.cpp \
vcsrc.cpp \
vcstr.cpp \
vcwserr.cpp \
vcwsock.cpp \
vcxstrm.cpp \
vcmemfun.cpp \
vcmemsel.cpp \
vcobj.cpp \
vcfac.cpp \
vclhfac.cpp \
vcmath.cpp \
vclhsys.cpp \
vcfile.cpp \
vcsfile.cpp  \
enc.cpp \
vccrypt2.cpp \
zcomp.cpp \
vcio.cpp \
vcuvsock.cpp \
lhuvsock.cpp \
vctrans.cpp \
vcudh.cpp \
vctsock.cpp


#LOCAL_STATIC_LIBRARIES := crypto5 zlib kazlib jenkins dwcls
LOCAL_C_INCLUDES := \
$(LOCAL_PATH)/../dwcls \
$(LOCAL_PATH)/../zlib \
$(LOCAL_PATH)/../kazlib \
$(LOCAL_PATH)/../jenkins \
$(LOCAL_PATH)/../crypto5 \
$(LOCAL_PATH)/../uv/include \
$(APP_PROJECT_PATH)

LOCAL_CPPFLAGS += -fpermissive -frtti


include $(BUILD_STATIC_LIBRARY)
