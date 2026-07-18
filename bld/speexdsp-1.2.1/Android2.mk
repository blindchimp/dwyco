
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libspeex-prebuilt
LOCAL_SRC_FILES := output/lib/libspeex.a
include $(PREBUILT_STATIC_LIBRARY)

