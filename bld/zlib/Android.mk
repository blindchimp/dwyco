LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE=zlib

LOCAL_CFLAGS := $(MY_CONF_LOCAL_CFLAGS) -DHAVE_UNISTD_H 

LOCAL_SRC_FILES :=  \
adler32.c compress.c crc32.c uncompr.c deflate.c trees.c \
zutil.c inflate.c inftrees.c inffast.c

include $(BUILD_STATIC_LIBRARY)

