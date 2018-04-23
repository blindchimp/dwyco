TEMPLATE = lib
CONFIG += staticlib
CONFIG -= qt

include($$PWD/../../$$DWYCO_CONFDIR/conf.pri)

INCLUDEPATH += 
DEFINES += NDEBUG FAST SASR NeedFunctionPrototypes=1 
macx-g++|linux-g++|linux-g++-64 {
#QMAKE_CXXFLAGS += -fpermissive
#QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-reorder
}

SOURCES = \
add.c \
code.c \
debug.c \
decode.c \
gsm_cr~1.c \
gsm_de~1.c \
gsm_de~2.c \
gsm_en~1.c \
gsm_ex~1.c \
gsm_im~1.c \
gsm_op~1.c \
gsm_pr~1.c \
long_t~1.c \
lpc.c \
prepro~1.c \
rpe.c \
short_~1.c \
table.c
