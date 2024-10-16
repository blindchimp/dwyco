
MY_CONF_LOCAL_PATH := $(call my-dir)
MY_CONF_LOCAL_CPPFLAGS := -DLINUX -DVCCFG_FILE -DUNIX
MY_CONF_LOCAL_CFLAGS := -DLINUX -DUNIX
include $(MY_CONF_LOCAL_PATH)/jhead/Android.mk
include $(MY_CONF_LOCAL_PATH)/kazlib/Android.mk
include $(MY_CONF_LOCAL_PATH)/dwcls/Android.mk
include $(MY_CONF_LOCAL_PATH)/jenkins/Android.mk
include $(MY_CONF_LOCAL_PATH)/crypto5/Android.mk
include $(MY_CONF_LOCAL_PATH)/vc/Android.mk
include $(MY_CONF_LOCAL_PATH)/pbm/Android.mk
include $(MY_CONF_LOCAL_PATH)/cdc32/Android.mk

include $(CLEAR_VARS)
LOCAL_MODULE := dwyco_jni
LOCAL_PATH := $(call my-dir)
LOCAL_LDLIBS := -llog

LOCAL_WHOLE_STATIC_LIBRARIES := jhead cdc32 pbm vc crypto5 kazlib jenkins dwcls
#LOCAL_SRC_FILES := foo.cpp
include $(BUILD_SHARED_LIBRARY)
