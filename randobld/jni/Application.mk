ifeq "$(NDK_ABI)" "arm"
APP_PROJECT_PATH := $(HOME)/git/dwyco/randobld
APP_ABI := armeabi-v7a
else
APP_PROJECT_PATH := $(HOME)/git/dwyco/randobld
APP_ABI := x86
endif
include $(APP_PROJECT_PATH)/dwyco_android.mk
APP_CPPFLAGS += -frtti -fexceptions -DANDROID -g -std=c++11
APP_CFLAGS += -DANDROID -g
APP_STL := c++_shared
#APP_MODULES := jhead cdc32 gsm lpc ppm pgm pbm vc crypto5 zlib kazlib jenkins dwcls
#APP_PLATFORM := android-20
#NDK_TOOLCHAIN_VERSION := 4.8
APP_PLATFORM := android-19
#NDK_TOOLCHAIN_VERSION := 4.9
APP_OPTIM := release
