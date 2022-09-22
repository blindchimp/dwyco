# WARNING for QT4 on macosx xcode 8/9: you need to change the mkspecs files that
# contain references to -mmacosx-version-min=... and remove them.
# otherwise you may get link errors related to old versions of libstdc++
# which is no longer supported.
# Also, if you get linker warnings related to version mismatches, you can
# hand it the Makefile generated for the top level cdcx build and
# change all occurances of "10.5" to "10.9"
VCCFG_COMP=$$PWD
DWYCOBG=1
DEFINES += VCCFG_FILE DWYCO_NO_UPNP
linux-*:DEFINES += LINUX
macx-ios-clang: DEFINES += LINUX MACOSX DWYCO_IOS
macx-*|linux-* {
QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-reorder -Wno-unused-variable -Wno-unused-function
QMAKE_CFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-reorder -Wno-unused-variable -Wno-unused-function
}
macx-*: DEFINES += LINUX MACOSX
win32 {
DEFINES += _WIN32 _CRT_SECURE_NO_WARNINGS __WIN32__ #_MBCS
DEFINES -= UNICODE
QMAKE_CXXFLAGS_WARN_ON -= -W3
QMAKE_CXXFLAGS += /wd4100 /wd4068 /wd4189 /wd4291
DEFINES += _Windows
DEFINES += CDCCORE_STATIC
DEFINES += DWYCO_NO_VIDEO_CAPTURE
}
linux-*|macx-*: QMAKE_CXX=ccache g++
android: DEFINES += ANDROID LINUX
linux-*:QMAKE_CXXFLAGS += -std=c++11

#QMAKE_CFLAGS += -fsanitize=address
#QMAKE_CXXFLAGS += -fsanitize=address

