
VCCFG_COMP=$$PWD
DWYCOBG=0
DWYCO_APP=cdcx
DEFINES += VCCFG_FILE DWYCO_APP_NICENAME=\\\"CDC-X\\\" #DWYCO_VC_THREADED

linux-* {
DEFINES += LINUX
# note: if you want audio, you have to use the qt5 audio drivers in qtdrv.
# don't use the VGQT video driver, use the v4l2 driver
FORCE_DESKTOP_VGQT=0
}
macx-ios-clang {
DEFINES += LINUX MACOSX DWYCO_IOS
QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-reorder -Wno-unused-variable -Wno-unused-function
QMAKE_CFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-reorder -Wno-unused-variable -Wno-unused-function
}
macx-* {
DEFINES += LINUX MACOSX DWYCO_FORCE_DESKTOP_VGQT
# we must use the vgqt qt5 driver since we don't have any native drivers.
FORCE_DESKTOP_VGQT=1
QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-reorder -Wno-unused-variable -Wno-unused-function
}

win32-* {
# use the old mtcapxe.dll native video drivers (forces you into 32bit everything.)
# audio drivers are build-in to cdcdll in this case.
FORCE_DESKTOP_VGQT=0
DEFINES += _WIN32 _CRT_SECURE_NO_WARNINGS __WIN32__ #_MBCS
DEFINES -= UNICODE
QMAKE_CXXFLAGS_WARN_ON -= -W3
QMAKE_CXXFLAGS += /wd4100 /wd4068 /wd4189 /wd4291
DEFINES += _Windows
}
linux-*|macx-* {
QMAKE_CXX=ccache g++

QMAKE_CFLAGS += -g #-fsanitize=address
QMAKE_CXXFLAGS += -g -std=c++11 #-fsanitize=address
QMAKE_LFLAGS += -g -std=c++11 #-fsanitize=address
}

