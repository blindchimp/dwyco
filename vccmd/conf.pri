LH_INTERPRETER=1

linux-g++*|macx-*:DEFINES += LINUX
linux-g++*|macx-g++*|macx-clang*: QMAKE_CXX=ccache g++

macx-*:DEFINES += MACOSX

#QMAKE_CFLAGS += -g -pg #-fsanitize=address
#QMAKE_CXXFLAGS += -g -pg #-fsanitize=address
QMAKE_CXXFLAGS += -std=c++11
#QMAKE_LFLAGS += -g -pg
#DEFINES += VC_DBG_ENVELOPE
QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-reorder

