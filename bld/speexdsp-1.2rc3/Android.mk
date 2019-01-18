LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libspeexdsp-prebuilt
LOCAL_SRC_FILES := output/lib/libspeexdsp.a
include $(PREBUILT_STATIC_LIBRARY)
