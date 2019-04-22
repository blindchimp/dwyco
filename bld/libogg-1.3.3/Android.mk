LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libogg-prebuilt
LOCAL_SRC_FILES := output/lib/libogg.a
include $(PREBUILT_STATIC_LIBRARY)
