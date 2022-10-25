VCCFG_COMP=$$PWD
DWYCO_APP=phoo
DWYCOBG=0
DWYCO_USE_LINUX_AUDIO=0
DEFINES += VCCFG_FILE DWYCO_APP_NICENAME=\\\"Phoo\\\" #DWYCO_VC_THREADED

CONFIG(debug, debug|release): DEFINES += DWYCO_DEBUG

linux-*|wasm-emscripten {
DEFINES += LINUX
QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-reorder -Wno-unused-variable -Wno-unused-function
QMAKE_CFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-reorder -Wno-unused-variable -Wno-unused-function
}

macx-ios-clang {
DEFINES += LINUX MACOSX DWYCO_IOS
QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-reorder -Wno-unused-variable -Wno-unused-function
}
macx-g++*|macx-clang* {
QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-reorder -Wno-unused-variable -Wno-unused-function
DEFINES += LINUX MACOSX
}

win32-* {
DEFINES += _WIN32 __WIN32__ #_MBCS
DEFINES -= UNICODE
QMAKE_CXXFLAGS_WARN_ON -= -W3
QMAKE_CXXFLAGS += /wd4100 /wd4068 /wd4189 /wd4291
DEFINES += _Windows
}

linux-g++*|macx-g++*: QMAKE_CXX=ccache g++
linux-clang*|macx-clang*: QMAKE_CXX=ccache clang

android-* {
DEFINES += ANDROID LINUX
QMAKE_CXXFLAGS += -frtti -fexceptions
QMAKE_CFLAGS += -frtti -fexceptions
QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-reorder -Wno-unused-variable -Wno-unused-function
}

wasm-emscripten {
DEFINES += EMSCRIPTEN
}

#QMAKE_CFLAGS += -fsanitize=address
#QMAKE_CXXFLAGS += -fsanitize=address

