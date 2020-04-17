TEMPLATE = app
FORCE_DESKTOP_VGQT=0

DEFINES += NO_BUILDTIME

QT += core qml multimedia network
QT += quickcontrols2
QT += location positioning

android: QT += androidextras
macx-clang: QT += macextras
CONFIG += c++11

DEFINES += DWYCO_APP_DEBUG
macx-ios-clang {
QMAKE_INFO_PLIST=Info.plist.ios
VERSION=1.0.0
QMAKE_TARGET_BUNDLE_PREFIX=com.dwyco
QMAKE_TARGET_BUNDLE=rando
}

macx-clang {
QMAKE_INFO_PLIST=Info.plist.mac
}

INCLUDEPATH += $${PWD}/../bld/qt-qml-models $${PWD}/../bld/qt-supermacros

#DEFINES += DWYCO_RELEASE
ICON=rando.icns
RC_FILE=rando.rc

SOURCES += main.cpp \
    dwyco_top.cpp \
    dwyco_new_msg.cpp \
    geospray.cpp \
    pfx.cpp \
    msglistmodel.cpp \
    msgpv.cpp \
    ssmap.cpp \
    notificationclient.cpp \
    convmodel.cpp \
    getinfo.cpp \
    qlimitedbuffer.cpp \
    resizeimage.cpp \
    androidperms.cpp

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/androidinst2

linux-* {
DEFINES += LINUX
DEFINES += DWYCO_APP_DEBUG

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
$${D}/kazlib/libkazlib.a \
$${D}/pbm/libpbm.a \
$${D}/jenkins/libjenkins.a \
$${D}/jhead/libjhead.a \
$${D}/qt-qml-models/libQtQmlModels_$${QT_ARCH}.a \
$${D}/libuv/libuv.a \
-lsqlite3

#PRE_TARGETDEPS += \
#$${D}/cdc32/libcdc32.a \
#$${D}/vc/libvc.a \
#$${D}/crypto5/libcrypto5.a \
#$${D}/dwcls/libdwcls.a \
#$${D}/kazlib/libkazlib.a \
#$${D}/pbm/libpbm.a \
#$${D}/jenkins/libjenkins.a \
#$${D}/jhead/libjhead.a \
#$${D}/qt-qml-models/libQtQmlModels.a \
#$${D}/libuv/libuv.a

}

wasm-emscripten {
DEFINES += LINUX
DEFINES += DWYCO_APP_DEBUG

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

D = $$OUT_PWD/../bld
SHADOW=$$OUT_PWD/..
LIBS += \
$${D}/cdc32/libcdc32.a \
$${D}/vc/libvc.a \
$${D}/crypto5/libcrypto5.a \
$${D}/dwcls/libdwcls.a \
$${D}/kazlib/libkazlib.a \
$${D}/pbm/libpbm.a \
$${D}/jenkins/libjenkins.a \
$${D}/jhead/libjhead.a \
$${D}/qt-qml-models/libQtQmlModels_$${QT_ARCH}.a \
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
$${D}/kazlib/libkazlib.a \
$${D}/pbm/libpbm.a \
$${D}/jenkins/libjenkins.a \
$${D}/jhead/libjhead.a \
$${D}/qt-qml-models/libQtQmlModels_$${QT_ARCH}.a

}

android-* {
DEFINES += LINUX VCCFG_FILE ANDROID

D = $${OUT_PWD}/../bld
L = $$PWD/../$$DWYCO_CONFDIR/libs/$$ANDROID_TARGET_ARCH
LIBS += $$D/qt-qml-models/libQtQmlModels_$${QT_ARCH}.a

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
include(functions.pri)

#QMAKE_EXTRA_TARGETS += $$copyAndroidSources("dwycojava", "src/com/dwyco/android", $$files($$PWD/../bld/android/com/dwyco/android/*.java))
QMAKE_EXTRA_TARGETS += $$copyAndroidSources("dwycojavasender", "src/com/dwyco/android", $$PWD/../bld/android/com/dwyco/android/DwycoSender.java)
QMAKE_EXTRA_TARGETS += $$copyAndroidSources("dwycojavaDwyco_msg", "src/com/dwyco/android", $$PWD/../bld/android/com/dwyco/android/Dwyco_Message.java)
QMAKE_EXTRA_TARGETS += $$copyAndroidSources("dwycojavastickyintent", "src/com/dwyco/android", $$PWD/../bld/android/com/dwyco/android/StickyIntentService.java)
QMAKE_EXTRA_TARGETS += $$copyAndroidSources("dwycojavaNotification", "src/com/dwyco/android", $$PWD/../bld/android/com/dwyco/android/NotificationClient.java)
QMAKE_EXTRA_TARGETS += $$copyAndroidSources("dwycojavaSocketLock", "src/com/dwyco/android", $$PWD/../bld/android/com/dwyco/android/SocketLock.java)
QMAKE_EXTRA_TARGETS += $$copyAndroidSources("dwycojavaMyFirebase", "src/com/dwyco/android", $$PWD/../bld/android/com/dwyco/android/MyFirebaseMessagingService.java)

QMAKE_EXTRA_TARGETS += $$copyAndroidSources("dwycojava2", "src/com/dwyco/cdc32", $$files($$PWD/../bld/android/com/dwyco/cdc32/*.java))
QMAKE_EXTRA_TARGETS += $$copyAndroidSources("dwycorandodeploy", ".", $$PWD/../../deploy-rando/google-services.json)


}

win32-msvc* {

DEFINES += MINGW_CLIENT VCCFG_FILE _CRT_SECURE_NO_WARNINGS __WIN32__ _Windows WIN32

# use this for linking to dynamic cdcdll
#INCLUDEPATH += dllwin
#LIBS +=  $${PWD}/cdcdll8.lib winmm.lib user32.lib kernel32.lib

# use this for linking with static cdc lib
DEFINES += CDCCORE_STATIC
# use this if you are building with qmake files
D = $$OUT_PWD\\..\\bld

#CONFIG(debug) {
#S=debug
#}

CONFIG(release) {
S=release
}

LIBS += \
$${D}\\cdc32\\$${S}\\cdc32.lib \
$${D}\\vc\\$${S}\\vc.lib \
$${D}\\crypto5\\$${S}\\crypto5.lib \
$${D}\\dwcls\\$${S}\\dwcls.lib \
$${D}\\kazlib\\$${S}\\kazlib.lib \
$${D}\\pbm\\$${S}\\pbm.lib \
$${D}\\jenkins\\$${S}\\jenkins.lib \
$${D}\\jhead\\$${S}\\jhead.lib \
$${D}\\qt-qml-models\\$${S}\\QtQmlModels.lib \
winmm.lib user32.lib kernel32.lib wsock32.lib vfw32.lib advapi32.lib ws2_32.lib  iphlpapi.lib binmode.obj

#delayimp.lib $${PWD}\\..\\bld\\mtcap\\mingw-rel\\win32\\mtcapxe.lib
#QMAKE_LFLAGS_RELEASE += /DELAYLOAD:mtcapxe.dll
#QMAKE_LFLAGS_DEBUG += /DELAYLOAD:mtcapxe.dll

#PRE_TARGETDEPS += \
#$${D}\\cdc32\\$${S}\\cdc32.lib \
#$${D}\\vc\\$${S}\\vc.lib \
#$${D}\\crypto5\\$${S}\\crypto5.lib \
#$${D}\\dwcls\\$${S}\\dwcls.lib \
#$${D}\\kazlib\\$${S}\\kazlib.lib \
#$${D}\\pbm\\$${S}\\pbm.lib \
#$${D}\\jenkins\\$${S}\\jenkins.lib \
#$${D}\\jhead\\$${S}\\jhead.lib \
#$${D}\\qt-qml-models\\$${S}\\QtQmlModels.lib

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
    geospray.h \
    msglistmodel.h \
    notificationclient.h \
    dwquerybymember.h \
    convmodel.h \
    getinfo.h \
    dwycolistscoped.h \
    qlimitedbuffer.h \
    resizeimage.h \
    androidperms.h

DISTFILES += \
    androidinst2/google-services.json \
    androidinst2/src/com/dwyco/cdc32/dwybg.java \
    androidinst2/src/com/dwyco/cdc32/dwybgJNI.java \
    androidinst2/src/com/dwyco/rando/app.java \
    androidinst2/src/com/dwyco/android/DwycoSender.java \
    androidinst2/src/com/dwyco/android/MyFirebaseMessagingService.java \
    androidinst2/src/com/dwyco/android/NotificationClient.java \
    androidinst2/src/com/dwyco/android/SocketLock.java \
    androidinst2/src/com/dwyco/rando/DwycoApp.java \
    androidinst2/AndroidManifest.xml \
    androidinst2/build.gradle \
    androidinst2/gradle/wrapper/gradle-wrapper.jar \
    androidinst2/gradle/wrapper/gradle-wrapper.properties \
    androidinst2/gradlew \
    androidinst2/gradlew.bat \
    androidinst2/res/values/libs.xml

#contains(ANDROID_TARGET_ARCH,x86) {
#    ANDROID_EXTRA_LIBS = $$PWD/../$$DWYCO_CONFDIR/libs/x86/libdwyco_jni.so
#}

#contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
#    ANDROID_EXTRA_LIBS = \
#        $$PWD/../$$DWYCO_CONFDIR/libs/armeabi-v7a/libdwyco_jni.so
#}
#contains(ANDROID_TARGET_ARCH,arm64-v8a) {
#    ANDROID_EXTRA_LIBS = \
#        $$PWD/../$$DWYCO_CONFDIR/libs/arm64-v8a/libdwyco_jni.so
#}

ANDROID_EXTRA_LIBS = $$PWD/../$$DWYCO_CONFDIR/libs/$${QT_ARCH}/libdwyco_jni_$${QT_ARCH}.so
