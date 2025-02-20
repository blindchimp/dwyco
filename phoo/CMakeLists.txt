cmake_minimum_required(VERSION 3.19)

project(phoo VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(DWYCO_CONFDIR phoo_cmake)
set(PHOO_VERSION "4.224")
set(PHOO_VERSION_NAME ${PHOO_VERSION})
set(PHOO_VERSION_CODE 2000120)
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
    ../bld/qt6drv
)
if(UNIX AND NOT APPLE AND NOT ANDROID)
    include_directories(../bld/v4lcap)
endif()

add_compile_definitions(NO_BUILDTIME)
add_compile_definitions(BUILDTIME="${PHOO_VERSION}")
add_compile_definitions($<$<CONFIG:Debug>:DWYCO_DEBUG>)


if(APPLE)
	add_compile_definitions(MAC_CLIENT)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)
qt_standard_project_setup(REQUIRES 6.8)
#find_package(QT NAMES Qt6  REQUIRED COMPONENTS Network)
find_package(Qt6 REQUIRED COMPONENTS Network)
#find_package(QT NAMES Qt6 REQUIRED COMPONENTS Multimedia)
find_package(Qt6 REQUIRED COMPONENTS Multimedia)

find_package(Qt6 REQUIRED COMPONENTS Core Gui StateMachine QuickControls2 Concurrent MultimediaWidgets)

# Get Qt version from the Qt6 config
execute_process(COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_VERSION OUTPUT_VARIABLE QT_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
add_compile_definitions(QT_VERSION_STRING="${QT_VERSION}")

# this needs a re-think, possibly a command to copy the androidinst3 file, then
# populate it with the java stuff would work. then change the package_source_dir to
# be the created folder. this could help us avoid the .gitignore mickey-mouse in the androidinst3
# dir. might look into a special cmake file
# that just does an install as well. for now, we just let it happen at cmake
# time instead of build time.
if(ANDROID)
file(COPY ../bld/android/com/dwyco DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/androidinst3/src/com/)
#file(COPY ../../deploy-phoo/google-services.json DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/androidinst3/)
endif()


set(SRCS
 main.cpp 

    ccmodel.cpp 
    discomodel.cpp 
    dwyco_top.cpp 
    dwyco_new_msg.cpp 
    joinlogmodel.cpp 
    pfx.cpp 
    msglistmodel.cpp 
    msgpv.cpp 
    simpledirmodel.cpp 
    ssmap.cpp 
    dwycoimageprovider.cpp 
    notificationclient.cpp 
    profpv.cpp 
    dwquerybymember.cpp 
    dvp.cpp 
    ct.cpp 
    callsm.cpp 
    chatlistmodel.cpp 
    dwycoprofilepreviewprovider.cpp 
    convmodel.cpp 
    getinfo.cpp 
    ignoremodel.cpp 
    qlimitedbuffer.cpp 
    resizeimage.cpp 
    simple_user_list.cpp 
    ctlist.cpp 
    dwycovideopreviewprovider.cpp  
    syncmodel.cpp
)

qt_add_resources(SRCS
    dwyco.qrc
    icons.qrc
)

if(APPLE)
    set(MACOSX_BUNDLE_ICON_FILE "greenguy.icns")
set(app_icon "${CMAKE_CURRENT_SOURCE_DIR}/greenguy.icns")
set_source_files_properties(${app_icon} PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources"
)
endif()

qt_add_executable(phoo
    MACOSX_BUNDLE ${app_icon}
   ${SRCS}
   callsm_objs.h # need this so automoc can find this header-only q_object


)
qt_add_ios_ffmpeg_libraries(phoo)


qt_import_plugins(phoo
    EXCLUDE_BY_TYPE qmltooling
)

qt_add_qml_module(phoo
    URI dwyco
    VERSION 1.0
# WARNING: without this option, it prepends
# some stupid ass "qt/qml/phoo_app" to all the
# resource paths
    NO_RESOURCE_TARGET_PATH
    QML_FILES
    main.qml
        SimpleChatBox.qml
        SimpDir.qml
        ProfileDialog.qml
        ProfileView.qml
        ProfileUpdateDialog.qml
        DeclarativeCamera.qml
        PhotoPreview.qml
        PhotoCaptureControls.qml
        CameraButton.qml
        MsgView.qml
        PublicChat.qml
        IgnoreListDialog.qml
        ChatList.qml
        AdminBrowse.qml
        UserActionMenu.qml
        ConvList.qml
        ForwardToList.qml
        PINDialog.qml
        PINChangeDialog.qml
        PicPreview.qml
        SimpleToolbar.qml
        CircularImage2.qml
        CQRes.qml
        ConvToolButton.qml
        MultiSelectToolbar.qml
        About.qml
        DSettings.qml
        FName.qml
        MsgReport.qml
        Warning.qml
        TextFieldX.qml
        VidCamPreview.qml
        CallButtons.qml
        DwycoVidRec.qml
        VidView.qml
        AppDrawer.qml
        AppDrawerForm.ui.qml
        SimpleMsgBrowse.qml
        VidCall.qml
        VidCallForm.ui.qml
        CallButtonLink.qml
        TipButton.qml
        +vgqt/VidCamPreview.qml
        SimpleGroupChat.qml
        SimpleTagMsgBrowse.qml
        GridToggle.qml
        +desktop/GridToggle.qml
        BareChatList.qml
        +desktop/PublicChat.qml
        MsgReview.qml
        DevGroup.qml
        BareConvList.qml
        +desktop/BareConvList.qml
        SendMulti.qml
        Reindex.qml
        RestoreAutoBackup.qml
        MessageYN.qml
        SimpleTrashBrowse.qml
        PulseLoader.qml
        MsgViewMenu.qml
        MsgViewMenuTrash.qml
           RESOURCES
           images/toolbutton.png
           images/toolbutton.sci

)

set(androidlibs
    ${CMAKE_CURRENT_SOURCE_DIR}/../phoobld/libs/arm64-v8a/libdwyco_jni.so
    ${CMAKE_CURRENT_SOURCE_DIR}/../phoobld/libs/armeabi-v7a/libdwyco_jni.so
    ${CMAKE_CURRENT_SOURCE_DIR}/../phoobld/libs/x86/libdwyco_jni.so
    ${CMAKE_CURRENT_SOURCE_DIR}/../phoobld/libs/x86_64/libdwyco_jni.so
)
# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(IOS)
    set(INFOPLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.cmake.ios)
else()
    set(INFOPLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.mac)
endif()

set_target_properties(phoo PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.dwyco.phoo
    MACOSX_BUNDLE_BUNDLE_VERSION "1.0.0"
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE_INFO_PLIST ${INFOPLIST}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/androidinst3
    QT_ANDROID_TARGET_SDK_VERSION 34

)
set_target_properties(phoo PROPERTIES
 QT_ANDROID_EXTRA_LIBS "${androidlibs}"
 QT_ANDROID_VERSION_NAME ${PHOO_VERSION_NAME}
 QT_ANDROID_VERSION_CODE ${PHOO_VERSION_CODE}

)

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
 #   zlib
    theora.1.2.x
    speex
    vorbis112
    ogg
    jenkins
    uv
    jhead
    miniupnpc
    qt6drv
    qt-qml-models
    Qt6::Quick
    Qt6::Network
    Qt6::Multimedia
    Qt6::QuickControls2
Qt6::Concurrent
Qt6::StateMachine
)

if(ANDROID)
    target_link_libraries(phoo PRIVATE
        qt-qml-models
        qt6drv
        ${CMAKE_CURRENT_SOURCE_DIR}/../phoobld/libs/${CMAKE_ANDROID_ARCH_ABI}/libdwyco_jni.so
        Qt6::Quick
        Qt6::Network
        Qt6::Multimedia
        Qt6::QuickControls2
	Qt6::Concurrent
        Qt6::StateMachine
    )
endif()

if(UNIX AND NOT ANDROID AND NOT APPLE)
target_link_libraries(phoo
    PRIVATE
   ${BASELIBS}
   #v4lcap
    -ldl
)
endif()

if(WIN32)
    target_link_libraries(phoo
        PRIVATE
       ${BASELIBS}
       winmm.lib user32.lib kernel32.lib wsock32.lib advapi32.lib ws2_32.lib  iphlpapi.lib psapi.lib
    )
target_link_options(phoo PRIVATE binmode.obj)
endif()

if(APPLE OR IOS)
    qt_add_ios_ffmpeg_libraries(phoo)
    target_link_libraries(phoo
        PRIVATE
       ${BASELIBS}
       Qt6::Widgets

    )
endif()

include(GNUInstallDirs)
install(TARGETS phoo
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_app_script(
    TARGET phoo
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})

