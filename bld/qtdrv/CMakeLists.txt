cmake_minimum_required(VERSION 3.14)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../${DWYCO_CONFDIR}/conf.cmake)

project(qtdrv LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Multimedia)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS MultimediaWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS MultimediaWidgets)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Concurrent)
include_directories(../miscsrc ../ppm ../pgm ../pbm ../cdc32)
add_library(qtdrv STATIC
    audi_qt.cpp
        audo_qt.cpp
        vgqt.cpp

)

target_link_libraries(qtdrv PRIVATE
    Qt${QT_VERSION_MAJOR}::Multimedia
     Qt${QT_VERSION_MAJOR}::Concurrent
     Qt${QT_VERSION_MAJOR}::MultimediaWidgets
)

target_compile_definitions(qtdrv PRIVATE QTDRV_LIBRARY)
