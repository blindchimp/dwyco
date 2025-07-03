
cmake_minimum_required(VERSION 3.14)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../../../${DWYCO_CONFDIR}/conf.cmake)

project(miniupnpc LANGUAGES C)
if(UNIX AND LINUX)
add_compile_options( -fPIC -Wall -W -Wstrict-prototypes -fno-common )
add_compile_definitions( MINIUPNPC_SET_SOCKET_TIMEOUT MINIUPNPC_GET_SRC_ADDR _BSD_SOURCE _DEFAULT_SOURCE _XOPEN_SOURCE=600)
endif()

if(WIN32)
add_compile_definitions(MINIUPNP_STATICLIB)
endif()


add_library(miniupnpc STATIC
miniwget.c minixml.c igd_desc_parse.c minisoap.c miniupnpc.c upnpreplyparse.c upnpcommands.c upnperrors.c connecthostport.c portlistingparse.c receivedata.c upnpdev.c minissdpc.c addr_is_reserved.c
)


#install(TARGETS miniupnpc DESTINATION lib)

