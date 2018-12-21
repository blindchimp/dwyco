LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libvorbisenc-prebuilt
LOCAL_SRC_FILES := output/lib/libvorbisenc.a
include $(PREBUILT_STATIC_LIBRARY)
