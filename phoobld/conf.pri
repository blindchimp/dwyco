VCCFG_COMP=$$PWD
DWYCOBG=0
DEFINES += VCCFG_FILE
linux-g++* {
DEFINES += LINUX
QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-reorder -Wno-unused-variable -Wno-unused-function
QMAKE_CFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-reorder -Wno-unused-variable -Wno-unused-function
}

macx-ios-clang: DEFINES += LINUX MACOSX DWYCO_IOS
macx-ios-clang: QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-reorder -Wno-unused-variable -Wno-unused-function
macx-g++*|macx-clang* {
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
QMAKE_CXXFLAGS_X86_64 += -mmacosx-version-min=10.9
}
macx-g++*|macx-clang*: DEFINES += LINUX MACOSX
win32-* {
DEFINES += _WIN32 __WIN32__ #_MBCS
DEFINES -= UNICODE
QMAKE_CXXFLAGS_WARN_ON -= -W3
QMAKE_CXXFLAGS += /wd4100 /wd4068 /wd4189 /wd4291
DEFINES += _Windows
}
linux-g++*|macx-g++*|macx-clang*: QMAKE_CXX=ccache g++
android-* {
DEFINES += ANDROID LINUX
QMAKE_CXXFLAGS += -frtti -fexceptions
QMAKE_CFLAGS += -frtti -fexceptions
QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-reorder -Wno-unused-variable -Wno-unused-function
}

#QMAKE_CFLAGS += -fsanitize=address
#QMAKE_CXXFLAGS += -fsanitize=address

