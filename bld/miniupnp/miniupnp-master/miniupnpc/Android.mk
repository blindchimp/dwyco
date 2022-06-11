LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := miniupnpc

LOCAL_CPPFLAGS := $(MY_CONF_LOCAL_CPPFLAGS) -Wall -W -Wstrict-prototypes -fno-common -DMINIUPNPC_SET_SOCKET_TIMEOUT -DMINIUPNPC_GET_SRC_ADDR -D_BSD_SOURCE -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=600

LOCAL_SRC_FILES := \
miniwget.c minixml.c igd_desc_parse.c minisoap.c miniupnpc.c upnpreplyparse.c upnpcommands.c upnperrors.c connecthostport.c portlistingparse.c receivedata.c upnpdev.c minissdpc.c addr_is_reserved.c

include $(BUILD_STATIC_LIBRARY)

