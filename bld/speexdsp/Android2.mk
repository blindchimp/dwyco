
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libspeex-prebuilt
LOCAL_SRC_FILES := $(APP_PROJECT_PATH)/obj/local/$(TARGET_ARCH_ABI)/libspeex.a
include $(PREBUILT_STATIC_LIBRARY)

