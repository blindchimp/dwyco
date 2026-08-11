cmake_minimum_required(VERSION 3.22.1)

# Plain C bridge. It deliberately has no C++ runtime dependency
# (ANDROID_STL=none); libdwyco_jni.so brings its own libc++_shared.so.
project(kfoo_native LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# kfoo project root (contains dlli.h)
set(KFOO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../..)
include_directories(${KFOO_ROOT})

# Prebuilt Dwyco core library, packaged under src/main/jniLibs/<abi>.
add_library(dwyco_jni SHARED IMPORTED)
set_target_properties(dwyco_jni PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libdwyco_jni.so
)

add_library(dwyco_kotlin_jni SHARED dwyco_bridge.c)

target_link_libraries(dwyco_kotlin_jni PRIVATE dwyco_jni log)
