LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libtheora-prebuilt
LOCAL_SRC_FILES := output/lib/libtheora.a
include $(PREBUILT_STATIC_LIBRARY)
