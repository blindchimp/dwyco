LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := jenkins

LOCAL_SRC_FILES :=  lookup2.c lookup3.c

LOCAL_CFLAGS := $(MY_CONF_LOCAL_CFLAGS)

include $(BUILD_STATIC_LIBRARY)
