
if(NOT ANDROID)
add_subdirectory(${CMAKE_SOURCE_DIR}/../bld/crypto5 ${CMAKE_CURRENT_BINARY_DIR}/bld/crypto5)
add_subdirectory(${CMAKE_SOURCE_DIR}/../bld/kazlib ${CMAKE_CURRENT_BINARY_DIR}/bld/kazlib)
add_subdirectory(${CMAKE_SOURCE_DIR}/../bld/jenkins ${CMAKE_CURRENT_BINARY_DIR}/bld/jenkins)
add_subdirectory(${CMAKE_SOURCE_DIR}/../bld/dwcls ${CMAKE_CURRENT_BINARY_DIR}/bld/dwcls)
add_subdirectory(${CMAKE_SOURCE_DIR}/../bld/pbm ${CMAKE_CURRENT_BINARY_DIR}/bld/pbm)
add_subdirectory(${CMAKE_SOURCE_DIR}/../bld/cdc32 ${CMAKE_CURRENT_BINARY_DIR}/bld/cdc32)
add_subdirectory(${CMAKE_SOURCE_DIR}/../bld/uv ${CMAKE_CURRENT_BINARY_DIR}/bld/uv)
add_subdirectory(${CMAKE_SOURCE_DIR}/../bld/vc ${CMAKE_CURRENT_BINARY_DIR}/bld/vc)
add_subdirectory(${CMAKE_SOURCE_DIR}/../bld/jhead ${CMAKE_CURRENT_BINARY_DIR}/bld/jhead)
endif()
add_subdirectory(${CMAKE_SOURCE_DIR}/../bld/qt-qml-models ${CMAKE_CURRENT_BINARY_DIR}/bld/qt-qml-models)

#add_subdirectory(${CMAKE_SOURCE_DIR}/../bld/miniupnp/miniupnp-master/miniupnpc ${CMAKE_CURRENT_BINARY_DIR}/bld/miniupnp/miniupnp-master/miniupnpc)

