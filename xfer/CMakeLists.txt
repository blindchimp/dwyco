

cmake_minimum_required(VERSION 3.14)

project(xfer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(${CMAKE_CURRENT_SOURCE_DIR}/../${DWYCO_CONFDIR}/conf.cmake)
# Create the executable
add_executable(xfer)
target_sources(xfer PRIVATE xfer.cc)

# Set the include directories
include_directories(
    ${VCCFG_COMP}
    /home/dwight/syncdev/keys
    ${CMAKE_CURRENT_SOURCE_DIR}/../bld/dwcls
    ${CMAKE_CURRENT_SOURCE_DIR}/../bld/vc
)

# Handle platform-specific configurations
if(UNIX)
    # Linux specific configurations
    set(LIBS
        ${CMAKE_CURRENT_SOURCE_DIR}/../bld/vc/libvc.a
        ${CMAKE_CURRENT_SOURCE_DIR}/../bld/dwcls/libdwcls.a
        ${CMAKE_CURRENT_SOURCE_DIR}/../bld/jenkins/libjenkins.a
        ${CMAKE_CURRENT_SOURCE_DIR}/../bld/kazlib/libkazlib.a
        ${CMAKE_CURRENT_SOURCE_DIR}/../bld/crypto5/libcrypto5.a
    )

    set(PRE_TARGETDEPS
        ${CMAKE_CURRENT_SOURCE_DIR}/../bld/vc/libvc.a
        ${CMAKE_CURRENT_SOURCE_DIR}/../bld/dwcls/libdwcls.a
        ${CMAKE_CURRENT_SOURCE_DIR}/../bld/jenkins/libjenkins.a
        ${CMAKE_CURRENT_SOURCE_DIR}/../bld/kazlib/libkazlib.a
        ${CMAKE_CURRENT_SOURCE_DIR}/../bld/crypto5/libcrypto5.a
    )

    add_link_options(-static)
endif()



# Link the required libraries
target_link_libraries(xfer ${LIBS})

# Handle pre-target dependencies
#add_dependencies(xfer ${PRE_TARGETDEPS})
