cmake_minimum_required(VERSION 3.5)

project(cdcx VERSION 0.1 LANGUAGES CXX)
set(DWYCO_CONFDIR cdcx_cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../${DWYCO_CONFDIR}/conf.cmake)
include(subdirs.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Qt6 REQUIRED COMPONENTS Core)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(QT NAMES Qt6  REQUIRED COMPONENTS Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Network)
find_package(QT NAMES Qt6 REQUIRED COMPONENTS Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Multimedia)
find_package(QT NAMES Qt6 REQUIRED COMPONENTS WebEngineCore)
#find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS WebEngine)
find_package(Qt6 REQUIRED COMPONENTS Core Gui WebEngineWidgets StateMachine)

add_compile_definitions(
UNICODE
VCCFG_FILE
CDCX_RELEASE
CDCX_MAC_USE_DEFAULT_LOCATION
WHATBOX=BrowseBox
#LOCAL_TEST
#NO_BUILDTIME
CDCX_WEBKIT
DWYCO_QT5
DWYCO_QT6
toAscii=toLatin1
#NO_DWYCO_AUDIO
#LEAK_CLEANUP
DWYCO_NICE_VERSION=\"4.107\"
)
include_directories(
./dllwin ../bld/qtdrv .
)

set(PROJECT_SOURCES
    main.cpp tfhex.cpp mainwin.cpp dwyco_new_msg.cpp evret.cpp dvp.cc about.cpp composer.cpp autoupdate.cpp msgtohtml.cpp player.cpp ssmap.cpp config.cpp snd.cpp croom.cpp pw.cpp login.cpp qwiz.cpp userwid.cpp cspw.cpp qval.cpp dirmisc.cpp iglist.cpp ct.cpp 
        vidlab.cpp 
        simple_public.cpp 
        popup_profile.cpp 
        lobbybox3.cpp 
        vidbox.cpp 
            toswarn.cpp 
        browsebox.cpp 
        msgbrowsedock.cpp 
        msglistmodel.cpp 
        profilepreviewbox.cpp 
        simpledirbox.cpp 
        simple_call.cpp 
        dwquerybymember.cpp 
        chattext.cpp 
        aswitch.cpp 
        adminw.cpp 
        vidsel.cpp 
            cdcxpan.cpp 
            pfx.cpp 
        prfview.cpp 
        profpv.cpp 
        simplehtmlbox.cpp 
        qlimitedbuffer.cpp 
        resizeimage.cpp 
        fname.cpp

        mainwin.ui about.ui composer.ui autoupdate.ui player.ui config.ui croom.ui pw.ui login.ui userwid.ui cspw.ui iglist.ui netspeedwid.ui  
            simple_public.ui 
            popup_profile.ui 
            lobbybox3.ui 
            vidbox.ui 
            toswarn.ui 
            browsebox.ui 
            msgbrowsedock.ui 
            simple_call.ui 
            adminw.ui 
            vidsel.ui 
            prfview.ui 
            simplehtmlbox.ui
            icons.qrc

)
if(APPLE)
    list(APPEND PROJECT_SOURCES mactards.mm)
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(cdcx
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET cdcx APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(cdcx SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(cdcx
            ${PROJECT_SOURCES}

        )
    add_dependencies(cdcx cdc32)
    endif()
endif()
set(D ${CMAKE_CURRENT_BINARY_DIR}/bld)
if(UNIX AND NOT APPLE)
target_link_libraries(cdcx PRIVATE
    ${D}/cdc32/libcdc32.a
    ${D}/vc/libvc.a
    ${D}/crypto5/libcrypto5.a
    ${D}/dwcls/libdwcls.a
    ${D}/gsm/libgsm.a
    ${D}/kazlib/libkazlib.a
    ${D}/ppm/libppm.a
    ${D}/pgm/libpgm.a
    ${D}/pbm/libpbm.a
    ${D}/zlib/libz.a
    ${D}/theora.1.2.x/libtheora.1.2.x.a
    ${D}/speex/libspeex.a
    ${D}/vorbis112/libvorbis112.a
    ${D}/ogg/libogg.a
    ${D}/jenkins/libjenkins.a
    ${D}/qt6drv/libqt6drv.a
    ${D}/uv/libuv.a
    ${D}/miniupnp/miniupnp-master/miniupnpc/libminiupnpc.a
    ${D}/v4lcap/libv4lcap.a
    -lv4l2
    -ldl
    -pthread

    Qt${QT_VERSION_MAJOR}::Widgets
Qt${QT_VERSION_MAJOR}::Network
Qt${QT_VERSION_MAJOR}::Multimedia
Qt6::WebEngineCore
    Qt6::WebEngineWidgets
    Qt6::StateMachine

)
elseif(APPLE)
    add_compile_definitions(MAC_CLIENT DWYCO_FORCE_DESKTOP_VGQT)
    find_library(CF CoreFoundation)
    find_library(COCOA Cocoa)
    target_link_libraries(cdcx PRIVATE
        ${D}/cdc32/libcdc32.a
        ${D}/vc/libvc.a
        ${D}/crypto5/libcrypto5.a
        ${D}/dwcls/libdwcls.a
        ${D}/gsm/libgsm.a
        ${D}/kazlib/libkazlib.a
        ${D}/ppm/libppm.a
        ${D}/pgm/libpgm.a
        ${D}/pbm/libpbm.a
        ${D}/zlib/libz.a
        ${D}/theora.1.2.x/libtheora.1.2.x.a
        ${D}/speex/libspeex.a
        ${D}/vorbis112/libvorbis112.a
        ${D}/ogg/libogg.a
        ${D}/jenkins/libjenkins.a
        ${D}/qt6drv/libqt6drv.a
        ${D}/uv/libuv.a
        ${D}/miniupnp/miniupnp-master/miniupnpc/libminiupnpc.a
        -ldl
        -pthread
        ${CF}
        ${COCOA}

        Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt6::WebEngineCore
        Qt6::WebEngineWidgets
        Qt6::StateMachine

    )
elseif(WIN32)

add_compile_definitions(  MINGW_CLIENT VCCFG_FILE _CRT_SECURE_NO_WARNINGS __WIN32__ _Windows WIN32)
add_compile_definitions(CDCCORE_STATIC)
# ssl either isn't available or doesn't work, so we just
# compile out parts that aren't available in qt4
add_compile_definitions(CDCX_NO_SSL)

target_link_libraries(
	cdcx PRIVATE
${D}/cdc32/cdc32.lib 
${D}/vc/vc.lib 
${D}/crypto5/crypto5.lib 
${D}/dwcls/dwcls.lib 
${D}/gsm/gsm.lib 
${D}/kazlib/kazlib.lib 
${D}/ppm/ppm.lib 
${D}/pgm/pgm.lib 
${D}/pbm/pbm.lib 
${D}/zlib/zlibstatic.lib
${D}/jenkins/jenkins.lib 
${D}/vorbis112/vorbis112.lib
${D}/theora.1.2.x/theora.1.2.x.lib 
${D}/speex/speex.lib 
${D}/ogg/ogg.lib 
${D}/miniupnp/miniupnp-master/miniupnpc/libminiupnpc.lib
${D}/qt6drv/qt6drv.lib
winmm.lib user32.lib kernel32.lib wsock32.lib advapi32.lib ws2_32.lib  iphlpapi.lib # binmode.obj
Qt${QT_VERSION_MAJOR}::Widgets
Qt${QT_VERSION_MAJOR}::Network
Qt${QT_VERSION_MAJOR}::Multimedia
Qt6::WebEngineCore
Qt6::WebEngineWidgets
Qt6::StateMachine
)
target_link_options(cdcx PRIVATE binmode.obj)

endif()

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER dwyco.cdc-x)
endif()
set_target_properties(cdcx PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.mac
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS cdcx
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(cdcx)
endif()
