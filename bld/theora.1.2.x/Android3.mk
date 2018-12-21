

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libtheoraenc-prebuilt
LOCAL_SRC_FILES := output/lib/libtheoraenc.a
include $(PREBUILT_STATIC_LIBRARY)
