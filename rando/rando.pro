TEMPLATE = app
FORCE_DESKTOP_VGQT=0

#macx-g++:dateincr.commands = ./dumptime-mac
#macx-clang:dateincr.commands = ./dumptime-mac
#macx-ios-clang:dateincr.commands = ./dumptime-mac
#linux-g++-64:dateincr.commands = ./dumptime
##linux-g++:dateincr.commands = ./dumptime32
#android:dateincr.commands = ./dumptime
#windows:dateincr.commands = .\\dumptime
#!macx-ios-clang:QMAKE_EXTRA_TARGETS += dateincr
#!macx-ios-clang:PRE_TARGETDEPS += dateincr
DEFINES += NO_BUILDTIME

CONFIG(appdir) {
target.path=/usr/bin
appdir_desktop.path=/usr/share/applications
appdir_desktop.files=phoo.desktop
appdir_icon.path=/usr/share/icons/hicolor/256x256/apps
appdir_icon.files=phoo.png
INSTALLS += appdir_icon appdir_desktop
}


QT += core qml quick multimedia network
QT += quickcontrols2

android: QT += androidextras
macx-clang: QT += macextras

DEFINES += DWYCO_APP_DEBUG
macx-ios-clang: QMAKE_INFO_PLIST=Info.plist.ios

INCLUDEPATH += $${PWD}/../bld/qt-qml-models $${PWD}/../bld/qt-supermacros

#DEFINES += DWYCO_RELEASE
ICON=rando.icns
RC_FILE=rando.rc

SOURCES += main.cpp \
    dwyco_top.cpp \
    dwyco_new_msg.cpp \
    pfx.cpp \
    msglistmodel.cpp \
    msgpv.cpp \
    ssmap.cpp \
    dwycoimageprovider.cpp \
    notificationclient.cpp \
    dwquerybymember.cpp \
    dvp.cpp \
    convmodel.cpp \
    getinfo.cpp \
    qlimitedbuffer.cpp \
    resizeimage.cpp

# note: you can *compile* the qt stuff on any platform, but
# as of 2017, the videoprobing stuff only works on android
# platforms. you must compile the dwyco core lib with NO_VIDEO_CAPTURE
# to remove the internal implementation of video capture (which is
# used only on desktop platforms)

DINC=$${PWD}/../bld

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/androidinst

linux-* {
DEFINES += LINUX
DEFINES += DWYCO_APP_DEBUG
equals(FORCE_DESKTOP_VGQT, 1) {
DEFINES += DWYCO_FORCE_DESKTOP_VGQT
}

QMAKE_CXXFLAGS += -g #-fsanitize=address #-O2
QMAKE_LFLAGS += -g #-fsanitize=address
linux-g++*:QMAKE_CXX=ccache g++
linux-clang*:QMAKE_CXX=ccache clang
QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-reorder -Wno-unused-variable -Wno-unused-function

SHADOW=$${OUT_PWD}
D = $${SHADOW}/../bld

LIBS += \
$${D}/cdc32/libcdc32.a \
$${D}/vc/libvc.a \
$${D}/crypto5/libcrypto5.a \
$${D}/dwcls/libdwcls.a \
$${D}/gsm/libgsm.a \
$${D}/kazlib/libkazlib.a \
$${D}/ppm/libppm.a \
$${D}/pgm/libpgm.a \
$${D}/pbm/libpbm.a \
$${D}/zlib/libzlib.a \
$${D}/theora.1.2.x/libtheora.1.2.x.a \
$${D}/vorbis112/libvorbis.a \
$${D}/ogg/libogg.a \
$${D}/jenkins/libjenkins.a \
$${D}/speex/libspeex.a \
$${D}/jhead/libjhead.a \
$${D}/v4lcap/libv4lcap.a \
$${D}/qt-qml-models/libQtQmlModels.a \
$${D}/libuv/libuv.a \
$${D}/miniupnp/miniupnp-master/miniupnpc/libminiupnpc.a \
-lsqlite3 \
-lv4l2

PRE_TARGETDEPS += \
$${D}/cdc32/libcdc32.a \
$${D}/vc/libvc.a \
$${D}/crypto5/libcrypto5.a \
$${D}/dwcls/libdwcls.a \
$${D}/gsm/libgsm.a \
$${D}/kazlib/libkazlib.a \
$${D}/ppm/libppm.a \
$${D}/pgm/libpgm.a \
$${D}/pbm/libpbm.a \
$${D}/zlib/libzlib.a \
$${D}/theora.1.2.x/libtheora.1.2.x.a \
$${D}/vorbis112/libvorbis.a \
$${D}/ogg/libogg.a \
$${D}/jenkins/libjenkins.a \
$${D}/speex/libspeex.a \
$${D}/jhead/libjhead.a \
$${D}/v4lcap/libv4lcap.a \
$${D}/qt-qml-models/libQtQmlModels.a \
$${D}/libuv/libuv.a

}

wasm-emscripten {
DEFINES += LINUX
DEFINES += DWYCO_APP_DEBUG
equals(FORCE_DESKTOP_VGQT, 1) {
DEFINES += DWYCO_FORCE_DESKTOP_VGQT
}
#INCLUDEPATH += $${DINC}/v4lcap

#QMAKE_CXXFLAGS += -g #-fsanitize=address #-O2
#QMAKE_LFLAGS += -g #-fsanitize=address

QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-reorder -Wno-unused-variable -Wno-unused-function
QMAKE_LFLAGS += -s ERROR_ON_UNDEFINED_SYMBOLS=0

SHADOW=$${OUT_PWD}
D = $${SHADOW}/../bld

LIBS += \
$${D}/cdc32/libcdc32.a \
$${D}/vc/libvc.a \
$${D}/crypto5/libcrypto5.a \
$${D}/dwcls/libdwcls.a \
$${D}/gsm/libgsm.a \
$${D}/kazlib/libkazlib.a \
$${D}/ppm/libppm.a \
$${D}/pgm/libpgm.a \
$${D}/pbm/libpbm.a \
$${D}/zlib/libzlib.a \
$${D}/theora/libtheora.a \
$${D}/vorbis112/libvorbis.a \
$${D}/ogg/libogg.a \
$${D}/jenkins/libjenkins.a \
$${D}/speex/libspeex.a \
$${D}/jhead/libjhead.a \
$${D}/qt-qml-models/libQtQmlModels.a

}

macx-clang {
DEFINES += MACOSX MAC_CLIENT
equals(FORCE_DESKTOP_VGQT, 1) {
DEFINES += DWYCO_FORCE_DESKTOP_VGQT
}
D = $$OUT_PWD/../bld
SHADOW=$$OUT_PWD/..
LIBS += \
$${D}/cdc32/libcdc32.a \
$${D}/vc/libvc.a \
$${D}/crypto5/libcrypto5.a \
$${D}/dwcls/libdwcls.a \
$${D}/gsm/libgsm.a \
$${D}/kazlib/libkazlib.a \
$${D}/ppm/libppm.a \
$${D}/pgm/libpgm.a \
$${D}/pbm/libpbm.a \
$${D}/zlib/libzlib.a \
$${D}/theora.1.2.x/libtheora.1.2.x.a \
$${D}/vorbis112/libvorbis.a \
$${D}/ogg/libogg.a \
$${D}/jenkins/libjenkins.a \
$${D}/speex/libspeex.a \
$${D}/jhead/libjhead.a \
$${D}/qt-qml-models/libQtQmlModels.a \
$${D}/miniupnp/miniupnp-master/miniupnpc/libminiupnpc.a \
$${PWD}/../bld/macdrv/libmacdrv.a \
$${D}/libuv/libuv.a \
-lsqlite3 \
-Wl,-framework,Cocoa -Wl,-framework,AudioToolbox -Wl,-framework,CoreAudio -Wl,-framework,QTKit -Wl,-framework,QuartzCore

}

macx-ios-clang {
DEFINES += DWYCO_IOS
D = $$OUT_PWD/../bld
LIBS += \
$${D}/cdc32/libcdc32.a \
$${D}/vc/libvc.a \
$${D}/crypto5/libcrypto5.a \
$${D}/dwcls/libdwcls.a \
$${D}/gsm/libgsm.a \
$${D}/kazlib/libkazlib.a \
$${D}/ppm/libppm.a \
$${D}/pgm/libpgm.a \
$${D}/pbm/libpbm.a \
$${D}/zlib/libzlib.a \
$${D}/theora.1.2.x/libtheora.1.2.x.a \
$${D}/vorbis112/libvorbis.a \
$${D}/ogg/libogg.a \
$${D}/jenkins/libjenkins.a \
$${D}/speex/libspeex.a \
$${D}/jhead/libjhead.a \
$${D}/miniupnp/miniupnp-master/miniupnpc/libminiupnpc.a \
$${D}/qt-qml-models/libQtQmlModels.a

}

android-* {
DEFINES += LINUX VCCFG_FILE CDCCORE_STATIC ANDROID

D = $${OUT_PWD}/../bld
equals(ANDROID_TARGET_ARCH, x86) {
L = $$PWD/../$$DWYCO_CONFDIR/libs/x86
} else {
L = $$PWD/../$$DWYCO_CONFDIR/libs/armeabi-v7a
}
LIBS += $$D/qt-qml-models/libQtQmlModels.a

# link against shared lib that is also used by the background, saves a bit of
# code but renders debugger useless. also NOTE: none of the JNI stuff will
# work in the main executable if it is statically linked. that is a
# limitation of java as far as i can tell.
LIBS += $${L}/libdwyco_jni.so
ANDROID_EXTRA_LIBS += $${L}/libdwyco_jni.so
#LIBS += \
#$${D}/libcdc32.a \
#$${D}/libvc.a \
#$${D}/libcrypto5.a \
#$${D}/libdwcls.a \
#$${D}/libgsm.a \
#$${D}/libkazlib.a \
#$${D}/liblpc.a \
#$${D}/libppm.a \
#$${D}/libpgm.a \
#$${D}/libpbm.a \
#$${D}/libzlib.a \
#$${D}/libtheoraenc.a \
#$${D}/libtheoradec.a \
#$${D}/libspeexdsp.a \
#$${D}/libvorbis.a \
#$${D}/libvorbisenc.a \
#$${D}/libogg.a \
#$${D}/libjhead.a \
#$${D}/libjenkins.a #-lgcc

QMAKE_LFLAGS += -g

}

win32-msvc* {

DEFINES += USE_VFW  MINGW_CLIENT VCCFG_FILE _CRT_SECURE_NO_WARNINGS __WIN32__ _Windows WIN32
equals(FORCE_DESKTOP_VGQT, 1) {
DEFINES += DWYCO_FORCE_DESKTOP_VGQT
}
# use this for linking to dynamic cdcdll
#INCLUDEPATH += dllwin
#LIBS +=  $${PWD}/cdcdll8.lib winmm.lib user32.lib kernel32.lib

# use this for linking with static cdc lib
DEFINES += CDCCORE_STATIC
# use this if you are building with qmake files
D = $$OUT_PWD\\..\\bld

CONFIG(debug) {
S=debug
}

#CONFIG(release) {
#S=release
#}

LIBS += \
$${D}\\cdc32\\$${S}\\cdc32.lib \
$${D}\\vc\\$${S}\\vc.lib \
$${D}\\crypto5\\$${S}\\crypto5.lib \
$${D}\\dwcls\\$${S}\\dwcls.lib \
$${D}\\gsm\\$${S}\\gsm.lib \
$${D}\\kazlib\\$${S}\\kazlib.lib \
$${D}\\ppm\\$${S}\\ppm.lib \
$${D}\\pgm\\$${S}\\pgm.lib \
$${D}\\pbm\\$${S}\\pbm.lib \
$${D}\\zlib\\$${S}\\zlib.lib \
$${D}\\jenkins\\$${S}\\jenkins.lib \
$${D}\\vorbis112\\$${S}\\vorbis.lib \
$${D}\\theora.1.2.x\\$${S}\\theora.1.2.x.lib \
$${D}\\speex\\$${S}\\speex.lib \
$${D}\\ogg\\$${S}\\ogg.lib \
$${D}\\jhead\\$${S}\\jhead.lib \
$${D}\\qt-qml-models\\$${S}\\QtQmlModels.lib \
$${D}\\miniupnp\\miniupnp-master\\miniupnpc\\$${S}\\miniupnpc.lib \
winmm.lib user32.lib kernel32.lib wsock32.lib vfw32.lib advapi32.lib ws2_32.lib  iphlpapi.lib binmode.obj \
$${PWD}\\..\\bld\\mtcap\\mingw-rel\\win32\\mtcapxe.lib

#delayimp.lib $${PWD}\\..\\bld\\mtcap\\mingw-rel\\win32\\mtcapxe.lib
#QMAKE_LFLAGS_RELEASE += /DELAYLOAD:mtcapxe.dll
#QMAKE_LFLAGS_DEBUG += /DELAYLOAD:mtcapxe.dll

PRE_TARGETDEPS += \
$${D}\\cdc32\\$${S}\\cdc32.lib \
$${D}\\vc\\$${S}\\vc.lib \
$${D}\\crypto5\\$${S}\\crypto5.lib \
$${D}\\dwcls\\$${S}\\dwcls.lib \
$${D}\\gsm\\$${S}\\gsm.lib \
$${D}\\kazlib\\$${S}\\kazlib.lib \
$${D}\\ppm\\$${S}\\ppm.lib \
$${D}\\pgm\\$${S}\\pgm.lib \
$${D}\\pbm\\$${S}\\pbm.lib \
$${D}\\zlib\\$${S}\\zlib.lib \
$${D}\\jenkins\\$${S}\\jenkins.lib \
$${D}\\vorbis112\\$${S}\\vorbis.lib \
$${D}\\theora.1.2.x\\$${S}\\theora.1.2.x.lib \
$${D}\\speex\\$${S}\\speex.lib \
$${D}\\ogg\\$${S}\\ogg.lib \
$${D}\\jhead\\$${S}\\jhead.lib \
$${D}\\qt-qml-models\\$${S}\\QtQmlModels.lib \
$${D}\\miniupnp\\miniupnp-master\\miniupnpc\\$${S}\\miniupnpc.lib

#\\mk\\depot\\dwycore\\bld\\vorbis112\\win32\\vs2003\\libvorbis\\Debug\\libvorbis.lib \
#\\mk\\depot\\dwycore\\bld\\theora\\win32\\vs2008\\win32\\Debug\\libtheora_static.lib \
#\\mk\\depot\\dwycore\\bld\\speex\\win32\\vs2008\\Debug\\libspeex.lib \
#\\mk\\depot\\dwycore\\bld\\speex\\win32\\vs2008\\libspeexdsp\\Debug\\libspeexdsp.lib \
#\\mk\\depot\\dwycore\\bld\\ogg\\win32\\vs2003\\libogg\\Debug\\libogg.lib \

}

RESOURCES += qml.qrc \
    dwyco.qrc \
    icons.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH = $$PWD

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
	dwyco_top.h \
    msglistmodel.h \
    dwycoimageprovider.h \
    notificationclient.h \
    dwstr-qstring.h \
    dwstr.h \
    dwquerybymember.h \
    dvp.h \
    convmodel.h \
    getinfo.h \
    dwycolistscoped.h \
    qlimitedbuffer.h \
    resizeimage.h \

DISTFILES += \
    androidinst/gradle/wrapper/gradle-wrapper.jar \
    androidinst/AndroidManifest.xml \
    androidinst/res/values/libs.xml \
    androidinst/build.gradle \
    androidinst/gradle/wrapper/gradle-wrapper.properties \
    androidinst/gradlew \
    androidinst/gradlew.bat \
    androidinst/src/com/dwyco/rando/NotificationClient.java \
    androidinst/src/com/dwyco/rando/Push_Notification.java \
    androidinst/src/com/dwyco/rando/dwybg.java \
    androidinst/src/com/dwyco/rando/dwybgJNI.java \
    androidinst/src/com/dwyco/rando/Dwyco_Message.java \
    androidinst/src/com/dwyco/rando/StickyIntentService.java \
    androidinst/google-services.json \
    androidinst/src/com/dwyco/rando/SocketLock.java \
    androidinst/src/com/dwyco/rando/MyFirebaseMessagingService.java \
    androidinst/src/com/dwyco/rando/DwycoSender.java \
    androidinst/src/com/dwyco/rando/DwycoProbe.java

contains(ANDROID_TARGET_ARCH,x86) {
    ANDROID_EXTRA_LIBS = $$PWD/../$$DWYCO_CONFDIR/libs/x86/libdwyco_jni.so
}

contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
    ANDROID_EXTRA_LIBS = \
        $$PWD/../$$DWYCO_CONFDIR/libs/armeabi-v7a/libdwyco_jni.so \
        $$PWD/arm/libcrypto.so \
        $$PWD/arm/libssl.so
}
