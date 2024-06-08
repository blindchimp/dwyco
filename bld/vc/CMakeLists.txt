cmake_minimum_required(VERSION 3.14)

project(vc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../${DWYCO_CONFDIR}/conf.cmake)
include_directories(
    ${VCCFG_COMP}
    ${CMAKE_CURRENT_SOURCE_DIR}/../dwcls
    ${CMAKE_CURRENT_SOURCE_DIR}/../zlib
    ${CMAKE_CURRENT_SOURCE_DIR}/../crypto5
    ${CMAKE_CURRENT_SOURCE_DIR}/../kazlib
    ${CMAKE_CURRENT_SOURCE_DIR}/../jenkins
    ${CMAKE_CURRENT_SOURCE_DIR}/../uv/include
)


# Definitions
add_definitions(-DVC_INTERNAL -DDWYCO_NO_TSOCK -DUNIX)

if(WIN32)
    add_definitions(-D_CRT_NO_POSIX_ERROR_CODES)
endif()

if(LH_INTERPRETER EQUAL 1)
    add_definitions(-DLHOBJ -DPERFHACKS -DFUNCACHE)
else()
    add_definitions(-DNO_VCEVAL)
endif()

add_library(vc STATIC
    dwgrows.cpp 
    pmatch.cpp 
    vc.cpp 
    vc2.cpp 
    vc3.cpp 
    vcatomic.cpp 
    vccfun.cpp 
    vccomp.cpp 
    vcctx.cpp 
    vccvar.cpp 
    vcdbg.cpp 
    vcdbl.cpp 
    vcdecom.cpp 
    vcdeflt.cpp 
    vcenco.cpp 
    vcexcctx.cpp 
    vcfunc.cpp 
    vcfuncal.cpp 
    vcfunctx.cpp 
    vcfundef.cpp 
    vcint.cpp 
    vclex.cpp 
    vclh.cpp 
    vcmap.cpp 
    vcnil.cpp 
    vcregex.cpp 
    vcser.cpp 
    vcsock.cpp 
    vcsrc.cpp 
    vcstr.cpp 
    vcwserr.cpp 
    vcwsock.cpp 
    vcxstrm.cpp 
    vcmemfun.cpp 
    vcmemsel.cpp 
    vcobj.cpp 
    vcfac.cpp 
    vclhfac.cpp 
    vcmath.cpp 
    vclhsys.cpp 
    vcfile.cpp 
    vcsfile.cpp  
    enc.cpp 
    vccrypt2.cpp 
    zcomp.cpp 
    vcio.cpp 
    vcuvsock.cpp 
    lhuvsock.cpp 
    vctrans.cpp 
    vcudh.cpp 
    reobj.cpp 
    vctsock.cpp
    vclhnet.cpp
)

target_compile_definitions(vc PRIVATE VC_LIBRARY)
