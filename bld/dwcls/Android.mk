
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := dwcls
LOCAL_SRC_FILES := \
dwhash.cpp \
useful.cpp \
dwstr.cpp \
dwtree2.cpp \
dwvp.cpp \
dwdate.cpp \
dwhist2.cc \
uricodec.cpp \
dwcls_timer.cpp

LOCAL_CPP_EXTENSION := .cpp .cc
LOCAL_CPP_FEATURES := $(MY_CONF_LOCAL_CPP_FEATURES)
LOCAL_CPPFLAGS := $(MY_CONF_LOCAL_CPPFLAGS)
#LOCAL_STATIC_LIBRARIES := kazlib
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../kazlib

include $(BUILD_STATIC_LIBRARY)


