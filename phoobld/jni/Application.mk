APP_ABI := armeabi-v7a arm64-v8a x86 x86_64
APP_PROJECT_PATH := $(HOME)/git/dwyco/phoobld
include $(APP_PROJECT_PATH)/dwyco_android.mk
APP_CPPFLAGS += -frtti -fexceptions -DANDROID -std=c++11
APP_CFLAGS += -DANDROID 
APP_STL := c++_shared
APP_PLATFORM := android-24
APP_OPTIM := release
