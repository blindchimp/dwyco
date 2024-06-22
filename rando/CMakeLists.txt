cmake_minimum_required(VERSION 3.19)

project(rando VERSION 0.1 LANGUAGES CXX)
set(DWYCO_CONFDIR rando_cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../${DWYCO_CONFDIR}/conf.cmake)
include(subdirs.cmake)
include_directories(
    .
    ../bld/dwcls
    ../bld/qt-qml-models
    ../bld/qt-supermacros
)
add_compile_definitions(NO_BUILDTIME)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick)
find_package(QT NAMES Qt6  REQUIRED COMPONENTS Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Network)
find_package(QT NAMES Qt6 REQUIRED COMPONENTS Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Multimedia)

find_package(Qt6 REQUIRED COMPONENTS Core Gui StateMachine QuickControls2)
set(FORCE_DESKTOP_VGQT 1)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(rando
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

qt_add_qml_module(randoqml
    URI rando
    VERSION 1.0
    RESOURCES qml.qrc
)
qt_add_resources(rando
    dwyco.qrc
    icons.qrc
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(rando PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.apprando
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)
set(D ${CMAKE_CURRENT_BINARY_DIR}/bld)
target_link_libraries(rando
    PRIVATE
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
    Qt6::QuickControls2
    -ldl
)

include(GNUInstallDirs)
install(TARGETS rando
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
