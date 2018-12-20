LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libvorbisfile-prebuilt
LOCAL_SRC_FILES := output/lib/libvorbisfile.a
include $(PREBUILT_STATIC_LIBRARY)
