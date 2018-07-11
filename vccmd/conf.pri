LH_INTERPRETER=1

linux-g++*:DEFINES += LINUX 
linux-g++*|macx-g++*|macx-clang*: QMAKE_CXX=ccache g++

QMAKE_CFLAGS += -g -fsanitize=address
QMAKE_CXXFLAGS += -g -fsanitize=address
QMAKE_LFLAGS += -g -fsanitize=address
DEFINES += VC_DBG_ENVELOPE

