
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libvorbis-prebuilt
LOCAL_SRC_FILES := output/lib/libvorbis.a
include $(PREBUILT_STATIC_LIBRARY)

