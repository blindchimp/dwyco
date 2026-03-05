
if(NOT ANDROID)
add_subdirectory(${CMAKE_SOURCE_DIR}/../bld/crypto8 ${CMAKE_CURRENT_BINARY_DIR}/bld/crypto8)
add_subdirectory(${CMAKE_SOURCE_DIR}/../bld/kazlib ${CMAKE_CURRENT_BINARY_DIR}/bld/kazlib)
add_subdirectory(${CMAKE_SOURCE_DIR}/../bld/jenkins ${CMAKE_CURRENT_BINARY_DIR}/bld/jenkins)
add_subdirectory(${CMAKE_SOURCE_DIR}/../bld/dwcls ${CMAKE_CURRENT_BINARY_DIR}/bld/dwcls)
add_subdirectory(${CMAKE_SOURCE_DIR}/../bld/pbm ${CMAKE_CURRENT_BINARY_DIR}/bld/pbm)
add_subdirectory(${CMAKE_SOURCE_DIR}/../bld/cdc32 ${CMAKE_CURRENT_BINARY_DIR}/bld/cdc32)
add_subdirectory(${CMAKE_SOURCE_DIR}/../bld/uv ${CMAKE_CURRENT_BINARY_DIR}/bld/uv)
add_subdirectory(${CMAKE_SOURCE_DIR}/../bld/vc ${CMAKE_CURRENT_BINARY_DIR}/bld/vc)
endif()
add_subdirectory(${CMAKE_SOURCE_DIR}/../bld/jhead ${CMAKE_CURRENT_BINARY_DIR}/bld/jhead)
add_subdirectory(${CMAKE_SOURCE_DIR}/../bld/qt-qml-models ${CMAKE_CURRENT_BINARY_DIR}/bld/qt-qml-models)
# this would be nice, but doesn't work. there is something bleeding in from the surrounding
# Qt cmake stuff that is making the android api level in the NDK build wrong.
#
# this thing is not dependent on qt in any way.
# maybe i'll figure it out eventually...
#
# in the mean time, you'll just have to invoke cmake by hand using the shell scripts in
# rando_dynbld
#
#if(ANDROID)
#add_subdirectory(${CMAKE_SOURCE_DIR}/../rando_dynbld ${CMAKE_CURRENT_BINARY_DIR}/rando_dynbld)
#endif()


#add_subdirectory(${CMAKE_SOURCE_DIR}/../bld/miniupnp/miniupnp-master/miniupnpc ${CMAKE_CURRENT_BINARY_DIR}/bld/miniupnp/miniupnp-master/miniupnpc)

