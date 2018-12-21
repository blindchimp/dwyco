
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libtheoradec-prebuilt
LOCAL_SRC_FILES := output/lib/libtheoradec.a
include $(PREBUILT_STATIC_LIBRARY)

