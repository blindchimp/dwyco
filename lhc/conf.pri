LH_INTERPRETER=1

linux-*:DEFINES += LINUX CACHE_LOOKUPS
linux-g++*|macx-g++*: QMAKE_CXX=ccache g++
linux-clang-*|macx-clang-*: QMAKE_CXX=ccache clang++
macx-*:DEFINES += LINUX MACOSX
QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-reorder

QMAKE_CFLAGS += -g  -O3 #-pg #-fsanitize=address
QMAKE_CXXFLAGS += -g -O3 # -pg -std=c++11# -fsanitize=address
QMAKE_LFLAGS += -g -O3 #-pg -std=c++11 #-fsanitize=address
#DEFINES += VC_DBG_ENVELOPE

