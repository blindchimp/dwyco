LH_INTERPRETER=1

linux-*|macx-* {
DEFINES += LINUX
QMAKE_CXX=ccache g++
QMAKE_CC=ccache gcc
}

linux-*: DEFINES += LH_WRAP_SPREAD LH_WRAP_SQLITE3
macx-*: DEFINES +=  MACOSX LH_WRAP_SQLITE3

#QMAKE_CFLAGS += -g -pg #-fsanitize=address
#QMAKE_CXXFLAGS += -g -pg #-fsanitize=address
QMAKE_CXXFLAGS += -std=c++11
#QMAKE_LFLAGS += -g -pg
#DEFINES += VC_DBG_ENVELOPE
QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-reorder

