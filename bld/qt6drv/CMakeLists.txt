cmake_minimum_required(VERSION 3.14)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../${DWYCO_CONFDIR}/conf.cmake)

project(qt6drv LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Multimedia)
find_package(QT NAMES Qt6 REQUIRED COMPONENTS MultimediaWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS MultimediaWidgets)
find_package(QT NAMES Qt6 REQUIRED COMPONENTS Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Concurrent)
include_directories(../miscsrc ../ppm ../pgm ../pbm ../cdc32)
add_library(qt6drv STATIC
    audi_qt6.cpp
        audo_qt6.cpp
        vgqt6.cpp

)

target_link_libraries(qt6drv PRIVATE
    Qt${QT_VERSION_MAJOR}::Multimedia
     Qt${QT_VERSION_MAJOR}::Concurrent
     Qt${QT_VERSION_MAJOR}::MultimediaWidgets
)

target_compile_definitions(qt6drv PRIVATE QT6DRV_LIBRARY)
