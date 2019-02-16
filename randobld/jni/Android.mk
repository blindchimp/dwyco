
MY_CONF_LOCAL_PATH := $(call my-dir)
MY_CONF_LOCAL_CPPFLAGS := -DLINUX -DVCCFG_FILE
MY_CONF_LOCAL_CFLAGS := -DLINUX
include $(MY_CONF_LOCAL_PATH)/jhead/Android.mk
include $(MY_CONF_LOCAL_PATH)/kazlib/Android.mk
include $(MY_CONF_LOCAL_PATH)/dwcls/Android.mk
include $(MY_CONF_LOCAL_PATH)/jenkins/Android.mk
include $(MY_CONF_LOCAL_PATH)/zlib/Android.mk
#include $(MY_CONF_LOCAL_PATH)/gsm/Android.mk
include $(MY_CONF_LOCAL_PATH)/crypto5/Android.mk
include $(MY_CONF_LOCAL_PATH)/vc/Android.mk
include $(MY_CONF_LOCAL_PATH)/pbm/Android.mk
include $(MY_CONF_LOCAL_PATH)/pgm/Android.mk
include $(MY_CONF_LOCAL_PATH)/ppm/Android.mk
include $(MY_CONF_LOCAL_PATH)/cdc32/Android.mk
#include $(MY_CONF_LOCAL_PATH)/libogg/Android.mk
#include $(MY_CONF_LOCAL_PATH)/libtheora/Android.mk
#include $(MY_CONF_LOCAL_PATH)/libtheora/Android2.mk
#include $(MY_CONF_LOCAL_PATH)/libtheora/Android3.mk
#include $(MY_CONF_LOCAL_PATH)/libvorbis/Android.mk
#include $(MY_CONF_LOCAL_PATH)/libvorbis/Android2.mk
#include $(MY_CONF_LOCAL_PATH)/libvorbis/Android3.mk
#include $(MY_CONF_LOCAL_PATH)/libspeex/Android.mk

include $(CLEAR_VARS)
LOCAL_MODULE := dwyco_jni
LOCAL_PATH := $(call my-dir)

LOCAL_WHOLE_STATIC_LIBRARIES := jhead cdc32 ppm pgm pbm vc crypto5 zlib kazlib jenkins dwcls
#LOCAL_SRC_FILES := foo.cpp
include $(BUILD_SHARED_LIBRARY)
