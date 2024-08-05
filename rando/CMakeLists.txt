cmake_minimum_required(VERSION 3.19)

project(rando VERSION 0.1 LANGUAGES CXX)
set(DWYCO_CONFDIR rando_cmake)
set(RANDO_VERSION "4.101")
set(RANDO_VERSION_NAME ${RANDO_VERSION})
set(RANDO_VERSION_CODE 3000149)
if (ANDROID)
    include(${ANDROID_SDK_ROOT}/android_openssl/CMakeLists.txt)
endif()
include(${CMAKE_CURRENT_SOURCE_DIR}/../${DWYCO_CONFDIR}/conf.cmake)
include(subdirs.cmake)
include_directories(
    .
    ../bld/dwcls
    ../bld/qt-qml-models
    ../bld/qt-supermacros
)
add_compile_definitions(NO_BUILDTIME)
add_compile_definitions(BUILDTIME="${RANDO_VERSION}")
add_compile_definitions($<$<CONFIG:Debug>:DWYCO_DEBUG>)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick)
find_package(QT NAMES Qt6  REQUIRED COMPONENTS Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Network)
find_package(QT NAMES Qt6 REQUIRED COMPONENTS Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Multimedia)

find_package(Qt6 REQUIRED COMPONENTS Core Gui StateMachine QuickControls2Material Positioning Location)

# this needs a re-think, possibly a command to copy the androidinst3 file, then
# populate it with the java stuff would work. then change the package_source_dir to
# be the created folder. this could help us avoid the .gitignore mickey-mouse in the androidinst3
# dir. might look into a special cmake file
# that just does an install as well. for now, we just let it happen at cmake
# time instead of build time.
if(ANDROID)
file(COPY ../bld/android/com/dwyco DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/androidinst3/src/com/)
file(COPY ../../deploy-rando/google-services.json DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/androidinst3/)
endif()
qt_standard_project_setup(REQUIRES 6.5)

set(SRCS
    main.cpp
    dwyco_top.cpp
        dwyco_new_msg.cpp
        geospray.cpp
        pfx.cpp
        msglistmodel.cpp
        msgpv.cpp
        ssmap.cpp
        notificationclient.cpp
        convmodel.cpp
        getinfo.cpp
        qlimitedbuffer.cpp
        resizeimage.cpp
        androidperms.cpp
)
qt_add_resources(SRCS
    dwyco.qrc
    icons.qrc
)
qt_add_executable(rando
   ${SRCS}
)

qt_add_qml_module(rando
    URI rando_app
    VERSION 1.0
# WARNING: without this option, it prepends
# some stupid ass "qt/qml/rando_app" to all the
# resource paths
    NO_RESOURCE_TARGET_PATH
    QML_FILES
    main.qml
           ProfileDialog.qml
           DeclarativeCamera.qml
           PhotoPreview.qml
           PhotoCaptureControls.qml
           CameraButton.qml
           MsgView.qml
           PicPreview.qml
           SimpleToolbar.qml
           CircularImage.qml
           #MultiSelectToolbar.qml
           About.qml
           DSettings.qml
           AppDrawer.qml
           AppDrawerForm.ui.qml
           TipButton.qml
           GridToggle.qml
           +desktop/GridToggle.qml
           FName.qml
           SimpleMsgList.qml
           MsgReport.qml
           Warning.qml
           ProgressRound.qml
           Help.qml
           MapImage.qml
           GeoListView.qml
           Reindex.qml
           RestoreAutoBackup.qml
           MessageYN.qml
           RESOURCES
           images/toolbutton.png
           images/toolbutton.sci

)

set(androidlibs
    ${CMAKE_CURRENT_SOURCE_DIR}/../randobld/libs/arm64-v8a/libdwyco_jni.so
    ${CMAKE_CURRENT_SOURCE_DIR}/../randobld/libs/armeabi-v7a/libdwyco_jni.so
    ${CMAKE_CURRENT_SOURCE_DIR}/../randobld/libs/x86/libdwyco_jni.so
    ${CMAKE_CURRENT_SOURCE_DIR}/../randobld/libs/x86_64/libdwyco_jni.so
)
# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(IOS)
    set(INFOPLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.cmake.ios)
else()
    set(INFOPLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.mac)
endif()

set_target_properties(rando PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.dwyco.rando
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE_INFO_PLIST ${INFOPLIST}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/androidinst3
    QT_ANDROID_TARGET_SDK_VERSION 34

)
set_target_properties(rando PROPERTIES
 QT_ANDROID_EXTRA_LIBS "${androidlibs}"
 QT_ANDROID_VERSION_NAME ${RANDO_VERSION_NAME}
 QT_ANDROID_VERSION_CODE ${RANDO_VERSION_CODE}

)

set(BASELIBS
    cdc32
    vc
    crypto5
    dwcls
    kazlib
    pbm
    uv
    jenkins
    qt-qml-models
    jhead
    Qt6::Quick
    Qt6::Network
    Qt6::Multimedia
    Qt6::QuickControls2Material
    Qt6::Positioning
    Qt6::Location
)

qt_import_plugins(rando
    EXCLUDE_BY_TYPE qmltooling
)

if(ANDROID)
    target_link_libraries(rando PRIVATE
        qt-qml-models
        ${CMAKE_CURRENT_SOURCE_DIR}/../randobld/libs/${CMAKE_ANDROID_ARCH_ABI}/libdwyco_jni.so
        Qt6::Quick
        Qt6::Network
        Qt6::Multimedia
        Qt6::QuickControls2Material
        Qt6::Positioning
        Qt6::Location
    )
endif()

if(UNIX AND NOT ANDROID)
target_link_libraries(rando
    PRIVATE
   ${BASELIBS}
    -ldl
)
endif()

if(WIN32)
    target_link_libraries(rando
        PRIVATE
       ${BASELIBS}
       winmm.lib user32.lib kernel32.lib wsock32.lib advapi32.lib ws2_32.lib  iphlpapi.lib
    )
target_link_options(rando PRIVATE binmode.obj)
endif()

if(IOS)
    target_link_libraries(rando
        PRIVATE
       ${BASELIBS}

    )
endif()

include(GNUInstallDirs)
install(TARGETS rando
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_app_script(
    TARGET rando
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})

