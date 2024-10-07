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

find_package(QT NAMES Qt6)
find_package(Qt6 REQUIRED COMPONENTS
    Core Gui WebEngineWidgets StateMachine Concurrent MultimediaWidgets
    Widgets Network Multimedia WebEngineCore)


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
./dllwin ../bld/qt6drv ../bld/dwcls .
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

set(BASELIBS
    cdc32
    vc
    crypto5
    dwcls
    gsm
    kazlib
    ppm
    pgm
    pbm
    zlib
    theora.1.2.x
    speex
    vorbis112
    ogg
    jenkins
    qt6drv
    uv
    miniupnpc
    Qt6::Widgets
Qt6::Network
Qt6::Multimedia
Qt6::WebEngineCore
    Qt6::WebEngineWidgets
    Qt6::StateMachine

)

set(D ${CMAKE_CURRENT_BINARY_DIR}/bld)
if(UNIX AND NOT APPLE)
target_link_libraries(cdcx PRIVATE
    ${BASELIBS}
    v4lcap
    -lv4l2
    -ldl
    -pthread
)
elseif(APPLE)
    add_compile_definitions(MAC_CLIENT DWYCO_FORCE_DESKTOP_VGQT)
    find_library(CF CoreFoundation)
    find_library(COCOA Cocoa)
    target_link_libraries(cdcx PRIVATE
	${BASELIBS}
        -ldl
        -pthread
        ${CF}
        ${COCOA}
    )
elseif(WIN32)

add_compile_definitions(  MINGW_CLIENT VCCFG_FILE _CRT_SECURE_NO_WARNINGS __WIN32__ _Windows WIN32)
add_compile_definitions(CDCCORE_STATIC)
# ssl either isn't available or doesn't work, so we just
# compile out parts that aren't available in qt4
add_compile_definitions(CDCX_NO_SSL)

target_link_libraries(
    cdcx PRIVATE
    ${BASELIBS}
    winmm.lib user32.lib kernel32.lib wsock32.lib advapi32.lib ws2_32.lib  iphlpapi.lib # binmode.obj
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

if(NOT APPLE)
install(
    DIRECTORY deploy/ DESTINATION bin
)
endif()

# Generate the deployment script for the target
qt_generate_deploy_app_script(
    TARGET cdcx
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
    #NO_TRANSLATIONS

)

# Call the deployment script on "cmake --install".
install(SCRIPT ${deploy_script})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(cdcx)
endif()
