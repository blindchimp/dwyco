LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := jhead

LOCAL_SRC_FILES :=   \
exif.cpp \
gpsinfo.cpp \
iptc.cpp \
jhead.cpp \
jpgfile.cpp \
jpgqguess.cpp \
makernote.cpp \
paths.cpp 

LOCAL_CFLAGS := $(MY_CONF_LOCAL_CFLAGS) -fpermissive
LOCAL_C_INCLUDES := $(LOCAL_PATH)/cjhead

include $(BUILD_STATIC_LIBRARY)
