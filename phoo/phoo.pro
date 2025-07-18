TEMPLATE = app
FORCE_DESKTOP_VGQT=1

include($$PWD/../$$DWYCO_CONFDIR/conf.pri)

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
greaterThan(QT_MAJOR_VERSION, 5) {
VER="4.213"
} else {
VER="3.213"
}
# i'll shit myself if this works on all platforms
DEFINES += BUILDTIME=\"\\\"$${VER}\\\"\"
DEFINES += QT_VERSION_STRING=\\\"$$QT_VERSION\\\"

CONFIG(appdir) {
target.path=/usr/bin
appdir_desktop.path=/usr/share/applications
appdir_desktop.files=phoo.desktop
appdir_icon.path=/usr/share/icons/hicolor/256x256/apps
appdir_icon.files=phoo.png
INSTALLS += appdir_icon appdir_desktop
}


QT += core qml quick multimedia network #widgets #positioning
QT += quickcontrols2
QT += statemachine
#QT += testlib
#DEFINES += DWYCO_MODEL_TEST
# someday, linux static build? fuck me
#QTPLUGIN.platforms = qminimal qeglfs
#CONFIG += static


!equals(QT_MAJOR_VERSION, 6) {
android: QT += androidextras
macx-clang|macx-g++: QT += macextras
INCLUDEPATH += $${PWD}/../bld/qtdrv
} else {
INCLUDEPATH += $${PWD}/../bld/qt6drv
}

QT += concurrent
DEFINES += DWYCO_APP_DEBUG
macx-ios-clang: QMAKE_INFO_PLIST=Info.plist.ios
macx-clang|macx-g++ {
QMAKE_INFO_PLIST=Info.plist.mac
#CONFIG -= app_bundle
}

INCLUDEPATH += $${PWD}/../bld/qt-qml-models $${PWD}/../bld/qt-supermacros $${PWD}/../bld/dwcls

#QMAKE_MAC_SDK = macosx10.9
DEFINES += DWYCO_RELEASE
ICON=greenguy.icns
RC_FILE=phoo.rc

SOURCES += main.cpp \
    androidperms.cpp \
    ccmodel.cpp \
    discomodel.cpp \
    dwyco_top.cpp \
    dwyco_new_msg.cpp \
    joinlogmodel.cpp \
    msgproxymodel.cpp \
    msgrawmodel.cpp \
    pfx.cpp \
    msgpv.cpp \
    simpledirmodel.cpp \
    ssmap.cpp \
    dwycoimageprovider.cpp \
    notificationclient.cpp \
    profpv.cpp \
    dwquerybymember.cpp \
    dvp.cpp \
    ct.cpp \
    callsm.cpp \
    chatlistmodel.cpp \
    dwycoprofilepreviewprovider.cpp \
    convmodel.cpp \
    getinfo.cpp \
    ignoremodel.cpp \
    qlimitedbuffer.cpp \
    resizeimage.cpp \
    simple_user_list.cpp \
    ctlist.cpp \
    dwycovideopreviewprovider.cpp  \
    syncmodel.cpp

# note: you can *compile* the qt stuff on any platform, but
# as of 2017, the videoprobing stuff only works on android
# platforms. you must compile the dwyco core lib with NO_VIDEO_CAPTURE
# to remove the internal implementation of video capture (which is
# used only on desktop platforms)

DINC=$${PWD}/../bld
equals(FORCE_DESKTOP_VGQT,1)|android-*|macx-ios-clang {
QT += concurrent
#SOURCES += vgqt.cpp
#HEADERS += vgqt.h
INCLUDEPATH += $${DINC}/kazlib $${DINC}/dwcls $${DINC}/pbm $${DINC}/pgm
}

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/androidinst3

linux-* {
DEFINES += LINUX
DEFINES += DWYCO_APP_DEBUG
equals(FORCE_DESKTOP_VGQT, 1) {
DEFINES += DWYCO_FORCE_DESKTOP_VGQT
}
INCLUDEPATH += $${DINC}/v4lcap

#QMAKE_CXXFLAGS += -g -fsanitize=address -O #-O2
#QMAKE_LFLAGS += -g -fsanitize=address -O
linux-g++*:QMAKE_CXX=ccache g++
linux-clang*:QMAKE_CXX=ccache clang
QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-reorder -Wno-unused-variable -Wno-unused-function

SHADOW=$${OUT_PWD}
D = $${SHADOW}/../bld

LIBS += \
$${D}/cdc32/libcdc32.a \
$${D}/vc/libvc.a \
$${D}/crypto8/libcrypto8.a \
$${D}/dwcls/libdwcls.a \
$${D}/gsm/libgsm.a \
$${D}/kazlib/libkazlib.a \
$${D}/ppm/libppm.a \
$${D}/pgm/libpgm.a \
$${D}/pbm/libpbm.a \
$${D}/theora.1.2.x/libtheora.1.2.x.a \
$${D}/vorbis112/libvorbis.a \
$${D}/ogg/libogg.a \
$${D}/jenkins/libjenkins.a \
$${D}/speex/libspeex.a \
$${D}/jhead/libjhead.a \
$${D}/v4lcap/libv4lcap.a \
$${D}/qt-qml-models/libQtQmlModels_$${QT_ARCH}.a \
$${D}/uv/libuv.a \
$${D}/miniupnp/miniupnp-master/miniupnpc/libminiupnpc.a \
$${D}/qt6drv/libqt6drv.a \
-lv4l2 \
-ldl

PRE_TARGETDEPS += \
$${D}/cdc32/libcdc32.a \
$${D}/vc/libvc.a \
$${D}/crypto8/libcrypto8.a \
$${D}/dwcls/libdwcls.a \
$${D}/gsm/libgsm.a \
$${D}/kazlib/libkazlib.a \
$${D}/ppm/libppm.a \
$${D}/pgm/libpgm.a \
$${D}/pbm/libpbm.a \
$${D}/theora.1.2.x/libtheora.1.2.x.a \
$${D}/vorbis112/libvorbis.a \
$${D}/ogg/libogg.a \
$${D}/jenkins/libjenkins.a \
$${D}/speex/libspeex.a \
$${D}/jhead/libjhead.a \
$${D}/v4lcap/libv4lcap.a \
$${D}/qt-qml-models/libQtQmlModels_$${QT_ARCH}.a \
$${D}/miniupnp/miniupnp-master/miniupnpc/libminiupnpc.a \
$${D}/uv/libuv.a \
$${D}/qt6drv/libqt6drv.a

}

wasm-emscripten {
DEFINES += LINUX
DEFINES += DWYCO_APP_DEBUG
DEFINES += NO_DWYCO_AUDIO
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
$${D}/crypto8/libcrypto8.a \
$${D}/dwcls/libdwcls.a \
$${D}/gsm/libgsm.a \
$${D}/kazlib/libkazlib.a \
$${D}/ppm/libppm.a \
$${D}/pgm/libpgm.a \
$${D}/pbm/libpbm.a \
$${D}/theora.1.2.x/libtheora.1.2.x.a \
$${D}/vorbis112/libvorbis.a \
$${D}/ogg/libogg.a \
$${D}/jenkins/libjenkins.a \
$${D}/speex/libspeex.a \
$${D}/jhead/libjhead.a \
$${D}/qt-qml-models/libQtQmlModels.a

}

macx-clang|macx-g++ {
DEFINES += MACOSX MAC_CLIENT
equals(FORCE_DESKTOP_VGQT, 1) {
DEFINES += DWYCO_FORCE_DESKTOP_VGQT
}
#QMAKE_CXXFLAGS += -stdlib=libc++
#QMAKE_LIBS += -lc++
#QMAKE_CXXFLAGS += -g -fsanitize=address #-O2
#QMAKE_LFLAGS += -g -fsanitize=address
D = $$OUT_PWD/../bld
SHADOW=$$OUT_PWD/..
LIBS += \
$${D}/cdc32/libcdc32.a \
$${D}/vc/libvc.a \
$${D}/crypto8/libcrypto8.a \
$${D}/dwcls/libdwcls.a \
$${D}/gsm/libgsm.a \
$${D}/kazlib/libkazlib.a \
$${D}/ppm/libppm.a \
$${D}/pgm/libpgm.a \
$${D}/pbm/libpbm.a \
$${D}/theora.1.2.x/libtheora.1.2.x.a \
$${D}/vorbis112/libvorbis.a \
$${D}/ogg/libogg.a \
$${D}/jenkins/libjenkins.a \
$${D}/speex/libspeex.a \
$${D}/jhead/libjhead.a \
$${D}/qt-qml-models/libQtQmlModels_$${QT_ARCH}.a \
$${D}/miniupnp/miniupnp-master/miniupnpc/libminiupnpc.a \
$${D}/qt6drv/libqt6drv.a \
$${D}/uv/libuv.a \
-lsqlite3 \
-Wl,-framework,Cocoa -Wl,-framework,AudioToolbox -Wl,-framework,CoreAudio -Wl,-framework,QTKit -Wl,-framework,QuartzCore

PRE_TARGETDEPS += \
$${D}/cdc32/libcdc32.a \
$${D}/vc/libvc.a \
$${D}/crypto8/libcrypto8.a \
$${D}/dwcls/libdwcls.a \
$${D}/gsm/libgsm.a \
$${D}/kazlib/libkazlib.a \
$${D}/ppm/libppm.a \
$${D}/pgm/libpgm.a \
$${D}/pbm/libpbm.a \
$${D}/theora.1.2.x/libtheora.1.2.x.a \
$${D}/vorbis112/libvorbis.a \
$${D}/ogg/libogg.a \
$${D}/jenkins/libjenkins.a \
$${D}/speex/libspeex.a \
$${D}/jhead/libjhead.a \
$${D}/qt-qml-models/libQtQmlModels_$${QT_ARCH}.a \
$${D}/miniupnp/miniupnp-master/miniupnpc/libminiupnpc.a \
$${D}/uv/libuv.a \
$${D}/qt6drv/libqt6drv.a


}

macx-ios-clang {
DEFINES += DWYCO_IOS
#QMAKE_CXXFLAGS += -stdlib=libc++
#QMAKE_LIBS += -lc++
#QTPLUGIN += qmlstatemachine
D = $$OUT_PWD/../bld
LIBS += \
$${D}/cdc32/libcdc32.a \
$${D}/vc/libvc.a \
$${D}/crypto8/libcrypto8.a \
$${D}/dwcls/libdwcls.a \
$${D}/gsm/libgsm.a \
$${D}/kazlib/libkazlib.a \
$${D}/ppm/libppm.a \
$${D}/pgm/libpgm.a \
$${D}/pbm/libpbm.a \
$${D}/theora.1.2.x/libtheora.1.2.x.a \
$${D}/vorbis112/libvorbis.a \
$${D}/ogg/libogg.a \
$${D}/jenkins/libjenkins.a \
$${D}/speex/libspeex.a \
$${D}/jhead/libjhead.a \
$${D}/miniupnp/miniupnp-master/miniupnpc/libminiupnpc.a \
$${D}/qt-qml-models/libQtQmlModels_$${QT_ARCH}.a \
$${D}/uv/libuv.a \
$${D}/qt6drv/libqt6drv.a

PRE_TARGETDEPS += \
$${D}/cdc32/libcdc32.a \
$${D}/vc/libvc.a \
$${D}/crypto8/libcrypto8.a \
$${D}/dwcls/libdwcls.a \
$${D}/gsm/libgsm.a \
$${D}/kazlib/libkazlib.a \
$${D}/ppm/libppm.a \
$${D}/pgm/libpgm.a \
$${D}/pbm/libpbm.a \
$${D}/theora.1.2.x/libtheora.1.2.x.a \
$${D}/vorbis112/libvorbis.a \
$${D}/ogg/libogg.a \
$${D}/jenkins/libjenkins.a \
$${D}/speex/libspeex.a \
$${D}/jhead/libjhead.a \
$${D}/qt-qml-models/libQtQmlModels_$${QT_ARCH}.a \
$${D}/miniupnp/miniupnp-master/miniupnpc/libminiupnpc.a \
$${D}/uv/libuv.a \
$${D}/qt6drv/libqt6drv.a
}

android-* {
DEFINES += LINUX VCCFG_FILE CDCCORE_STATIC ANDROID

D = $${OUT_PWD}/../bld

L=$$PWD/../$$DWYCO_CONFDIR/libs/$$ANDROID_TARGET_ARCH

LIBS += $$D/qt-qml-models/libQtQmlModels_$${QT_ARCH}.a $$D/qt6drv/libqt6drv_$${QT_ARCH}.a

# link against shared lib that is also used by the background, saves a bit of
# code but renders debugger useless. also NOTE: none of the JNI stuff will
# work in the main executable if it is statically linked. that is a
# limitation of java as far as i can tell.
LIBS += $${L}/libdwyco_jni.so
#ANDROID_EXTRA_LIBS += $${L}/libdwyco_jni.so
ANDROID_EXTRA_LIBS = $$PWD/../$$DWYCO_CONFDIR/libs/armeabi-v7a/libdwyco_jni.so $$PWD/../$$DWYCO_CONFDIR/libs/arm64-v8a/libdwyco_jni.so $$PWD/../$$DWYCO_CONFDIR/libs/x86/libdwyco_jni.so $$PWD/../$$DWYCO_CONFDIR/libs/x86_64/libdwyco_jni.so

ANDROID_TARGET_SDK_VERSION=34
#ANDROID_MIN_SDK_VERSION=24
ANDROID_VERSION_CODE=2000119
ANDROID_VERSION_NAME=$$VER
#contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
#    ANDROID_EXTRA_LIBS += \
#        $$PWD/arm/libcrypto.so \
#        $$PWD/arm/libssl.so
#}
#message($$QMAKE_HOST.os)
equals(QMAKE_HOST.os, Darwin) {
message(MACOS)
include(/Users/dwight/android/astudio/android_openssl/openssl.pri)
} else {
include(/home/dwight/Android/Sdk/android_openssl/openssl.pri)
}
#LIBS += \
#$${D}/libcdc32.a \
#$${D}/libvc.a \
#$${D}/libcrypto8.a \
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

include(functions.pri)

QMAKE_EXTRA_TARGETS += $$copyAndroidSources("dwycojava", "src/com/dwyco/android", $$files($$PWD/../bld/android/com/dwyco/android/*.java))
QMAKE_EXTRA_TARGETS += $$copyAndroidSources("dwycojava2", "src/com/dwyco/cdc32", $$files($$PWD/../bld/android/com/dwyco/cdc32/*.java))

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
D=$${OUT_PWD}/../bld
CONFIG(debug, debug|release) {
    S=debug
}
CONFIG(release, debug|release) {
    S=release
}

LIBS += \
$${D}/cdc32/$${S}/cdc32.lib \
$${D}/vc/$${S}/vc.lib \
$${D}/crypto8/$${S}/crypto8.lib \
$${D}/dwcls/$${S}/dwcls.lib \
$${D}/gsm/$${S}/gsm.lib \
$${D}/kazlib/$${S}/kazlib.lib \
$${D}/ppm/$${S}/ppm.lib \
$${D}/pgm/$${S}/pgm.lib \
$${D}/pbm/$${S}/pbm.lib \
$${D}/jenkins/$${S}/jenkins.lib \
$${D}/vorbis112/$${S}/vorbis.lib \
$${D}/theora.1.2.x/$${S}/theora.1.2.x.lib \
$${D}/speex/$${S}/speex.lib \
$${D}/ogg/$${S}/ogg.lib \
$${D}/jhead/$${S}/jhead.lib \
$${D}/uv/$${S}/uv.lib \
$${D}/qt6drv/$${S}/qt6drv.lib \
$${D}/qt-qml-models/$${S}/QtQmlModels_$${QT_ARCH}.lib \
$${D}/miniupnp/miniupnp-master/miniupnpc/$${S}/miniupnpc.lib \
winmm.lib user32.lib kernel32.lib wsock32.lib advapi32.lib ws2_32.lib  iphlpapi.lib psapi.lib binmode.obj \


PRE_TARGETDEPS += \
$${D}/cdc32/$${S}/cdc32.lib \
$${D}/vc/$${S}/vc.lib \
$${D}/crypto8/$${S}/crypto8.lib \
$${D}/dwcls/$${S}/dwcls.lib \
$${D}/gsm/$${S}/gsm.lib \
$${D}/kazlib/$${S}/kazlib.lib \
$${D}/ppm/$${S}/ppm.lib \
$${D}/pgm/$${S}/pgm.lib \
$${D}/pbm/$${S}/pbm.lib \
$${D}/jenkins/$${S}/jenkins.lib \
$${D}/vorbis112/$${S}/vorbis.lib \
$${D}/theora.1.2.x/$${S}/theora.1.2.x.lib \
$${D}/speex/$${S}/speex.lib \
$${D}/ogg/$${S}/ogg.lib \
$${D}/jhead/$${S}/jhead.lib \
$${D}/qt-qml-models/$${S}/QtQmlModels_$${QT_ARCH}.lib \
$${D}/miniupnp/miniupnp-master/miniupnpc/$${S}/miniupnpc.lib \
$${D}/qt6drv/$${S}/qt6drv.lib

}

RESOURCES += qml.qrc \
    dwyco.qrc \
    icons.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH = $$PWD

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    androidperms.h \
    ccmodel.h \
    discomodel.h \
	dwyco_top.h \
    joinlogmodel.h \
    dwycoimageprovider.h \
    msgproxymodel.h \
    msgrawmodel.h \
    notificationclient.h \
    profpv.h \
    dwquerybymember.h \
    dvp.h \
    ct.h \
    callsm_objs.h \
    callsm.h \
    chatlistmodel.h \
    dwycoprofilepreviewprovider.h \
    convmodel.h \
    getinfo.h \
    ignoremodel.h \
    qlimitedbuffer.h \
    resizeimage.h \
    simple_user_list.h \
    ctlist.h \
    dwycovideopreviewprovider.h \
    simpledirmodel.h \
    syncmodel.h

DISTFILES += \
    androidinst3/google-services.json \
    androidinst3/src/com/dwyco/cdc32/dwybg.java \
    androidinst3/src/com/dwyco/cdc32/dwybgJNI.java \
    androidinst3/src/com/dwyco/phoo/app.java \
    androidinst3/src/com/dwyco/android/DwycoProbe.java \
    androidinst3/src/com/dwyco/android/MyFirebaseMessagingService.java \
    androidinst3/src/com/dwyco/android/NotificationClient.java \
    androidinst3/src/com/dwyco/android/SocketLock.java \
    androidinst3/src/com/dwyco/android/FileUtils.java \
    androidinst3/src/com/dwyco/phoo/DwycoApp.java \
    androidinst3/AndroidManifest.xml \
    androidinst3/build.gradle \
    androidinst3/gradle.properties \
    androidinst3/gradle/wrapper/gradle-wrapper.jar \
    androidinst3/gradle/wrapper/gradle-wrapper.properties \
    androidinst3/gradlew \
    androidinst3/gradlew.bat \
    androidinst3/res/values/libs.xml \
    androidinst3/res/xml/provider_paths.xml

android: include(/home/dwight/Android/Sdk/android_openssl/openssl.pri)
