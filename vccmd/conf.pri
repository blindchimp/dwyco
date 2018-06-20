LH_INTERPRETER=1

linux-g++*:DEFINES += LINUX 
linux-g++*|macx-g++*|macx-clang*: QMAKE_CXX=ccache g++

#QMAKE_CFLAGS += -fsanitize=address
#QMAKE_CXXFLAGS += -fsanitize=address

