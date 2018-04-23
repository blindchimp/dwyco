# WARNING for QT4 on macosx xcode 8/9: you need to change the mkspecs files that
# contain references to -mmacosx-version-min=... and remove them.
# otherwise you may get link errors related to old versions of libstdc++
# which is no longer supported.
VCCFG_COMP=$$PWD
DEFINES += VCCFG_FILE
linux-g++*:DEFINES += LINUX UNIX
macx-ios-clang: DEFINES += LINUX MACOSX DWYCO_IOS
macx-ios-clang: QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-reorder -Wno-unused-variable -Wno-unused-function
macx-g++|macx-clang: DEFINES += LINUX MACOSX 
win32 {
DEFINES += _WIN32 _CRT_SECURE_NO_WARNINGS __WIN32__ #_MBCS
DEFINES -= UNICODE
QMAKE_CXXFLAGS_WARN_ON -= -W3
QMAKE_CXXFLAGS += /wd4100 /wd4068 /wd4189 /wd4291
DEFINES += _Windows
}
linux-g++*|macx-g++|macx-clang: QMAKE_CXX=ccache g++
android: DEFINES += ANDROID LINUX

#QMAKE_CFLAGS += -fsanitize=address
#QMAKE_CXXFLAGS += -fsanitize=address

