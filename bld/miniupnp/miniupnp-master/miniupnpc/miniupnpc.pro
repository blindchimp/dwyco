TEMPLATE = lib
CONFIG += staticlib
CONFIG -= qt
include($$PWD/../../../../$$DWYCO_CONFDIR/conf.pri)

QMAKE_CFLAGS += -fPIC -O -Wall -W -Wstrict-prototypes -fno-common -DMINIUPNPC_SET_SOCKET_TIMEOUT -DMINIUPNPC_GET_SRC_ADDR -D_BSD_SOURCE -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=600

SOURCES = \
miniwget.c minixml.c igd_desc_parse.c minisoap.c miniupnpc.c upnpreplyparse.c upnpcommands.c upnperrors.c connecthostport.c portlistingparse.c receivedata.c upnpdev.c minissdpc.c
