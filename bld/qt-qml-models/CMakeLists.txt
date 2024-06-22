cmake_minimum_required(VERSION 3.10)
project(qt-qml-models)

# Add static library
add_library(qt-qml-models STATIC)

set_target_properties(qt-qml-models PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)

# Specify source files
target_sources(qt-qml-models PRIVATE
    QQmlObjectListModel.cpp
    QQmlVariantListModel.cpp
    QQmlObjectListModel.h
    QQmlVariantListModel.h
    QtQmlTricksPlugin_SmartDataModels.h
)

# Specify include directories
target_include_directories(qt-qml-models PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Find Qt packages
find_package(Qt6 COMPONENTS Core Qml REQUIRED)

# Add dependencies
target_link_libraries(qt-qml-models PUBLIC
    Qt6::Core Qt6::Qml)
