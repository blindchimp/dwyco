# WARNING for QT4 on macosx xcode 8/9: you need to change the mkspecs files that
# contain references to -mmacosx-version-min=... and remove them.
# otherwise you may get link errors related to old versions of libstdc++
# which is no longer supported.
# Also, if you get linker warnings related to version mismatches, you can
# hand it the Makefile generated for the top level cdcx build and
# change all occurances of "10.5" to "10.9"
VCCFG_COMP=$$PWD
DWYCOBG=0
DEFINES += VCCFG_FILE
linux-g++*:DEFINES += LINUX 
macx-ios-clang {
DEFINES += LINUX MACOSX DWYCO_IOS
QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-reorder -Wno-unused-variable -Wno-unused-function
}
macx-ios-clang|macx-g++|macx-clang {
#QMAKE_CXXFLAGS += -stdlib=libc++
#QMAKE_LFLAGS += -stdlib=libstdc++
#QMAKE_MAC_SDK.macos.version=10.11
#QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
QMAKE_CXXFLAGS_X86_64 += -mmacosx-version-min=10.9
}
macx-g++|macx-clang: DEFINES += LINUX MACOSX
win32-* {
DEFINES += _WIN32 _CRT_SECURE_NO_WARNINGS __WIN32__ #_MBCS
DEFINES -= UNICODE
QMAKE_CXXFLAGS_WARN_ON -= -W3
QMAKE_CXXFLAGS += /wd4100 /wd4068 /wd4189 /wd4291
DEFINES += _Windows
DEFINES += CDCCORE_STATIC
}
linux-g++|linux-g++-64|macx-g++|macx-clang: QMAKE_CXX=ccache g++

QMAKE_CFLAGS += #-fsanitize=address
QMAKE_CXXFLAGS += -std=c++11 #-fsanitize=address
QMAKE_LFLAGS += -std=c++11 #-fsanitize=address

