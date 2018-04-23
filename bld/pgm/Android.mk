LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := pgm

LOCAL_CPPFLAGS := $(MY_CONF_LOCAL_CPPFLAGS) -DANDROID 


LOCAL_SRC_FILES=   \
	libpgm1.c \
	libpgm2.c


#LOCAL_STATIC_LIBRARIES := pbm crypto5 zlib kazlib jenkins dwcls
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../pbm
LOCAL_CPPFLAGS += -fpermissive -frtti


include $(BUILD_STATIC_LIBRARY)
