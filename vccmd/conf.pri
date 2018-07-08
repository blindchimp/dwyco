LH_INTERPRETER=1

linux-g++*:DEFINES += LINUX 
linux-g++*|macx-g++*|macx-clang*: QMAKE_CXX=ccache g++

#QMAKE_CFLAGS += -g -pg #-fsanitize=address
#QMAKE_CXXFLAGS += -g -pg #-fsanitize=address
#QMAKE_LFLAGS += -g -pg
#DEFINES += VC_DBG_ENVELOPE

