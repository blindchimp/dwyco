LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := pbm

LOCAL_CPPFLAGS := $(MY_CONF_LOCAL_CPPFLAGS) -DANDROID 


LOCAL_SRC_FILES=   \
	libpbm1.c \
	libpbm2.c \
	libpbm3.c \
	libpbm4.c \
	libpbm5.c


#LOCAL_STATIC_LIBRARIES := crypto5 zlib kazlib jenkins dwcls
#LOCAL_C_INCLUDES := $(LOCAL_PATH)/../dwcls $(LOCAL_PATH)/../zlib $(LOCAL_PATH)/../kazlib $(LOCAL_PATH)/../jenkins $(LOCAL_PATH)/../crypto5
LOCAL_CPPFLAGS += -fpermissive -frtti


include $(BUILD_STATIC_LIBRARY)
