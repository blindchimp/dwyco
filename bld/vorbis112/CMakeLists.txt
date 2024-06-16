
cmake_minimum_required(VERSION 3.14)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../${DWYCO_CONFDIR}/conf.cmake)

project(vorbis112 LANGUAGES C)
include_directories(include lib ../ogg/include)

if(UNIX)
set(SOURCES  
    lib/window.c 
    lib/vorbisenc.c 
    lib/synthesis.c 
    lib/smallft.c 
    lib/sharedbook.c 
    lib/res0.c 
    lib/registry.c 
    lib/psy.c 
    lib/mdct.c 
    lib/mapping0.c 
    lib/lsp.c 
    lib/lpc.c 
    lib/lookup.c 
    lib/info.c 
    lib/floor1.c 
    lib/floor0.c 
    lib/envelope.c 
    lib/codebook.c 
    lib/block.c 
    lib/bitrate.c 
    lib/analysis.c
)
endif()

if(APPLE)
endif()

if(WIN32)
add_compile_options(/wd4244 /wd4305)
set(SOURCES  
    lib/window.c 
    lib/vorbisenc.c 
    lib/synthesis.c 
    lib/smallft.c 
    lib/sharedbook.c 
    lib/res0.c 
    lib/registry.c 
    lib/psy.c 
    lib/mdct.c 
    lib/mapping0.c 
    lib/lsp.c 
    lib/lpc.c 
    lib/lookup.c 
    lib/info.c 
    lib/floor1.c 
    lib/floor0.c 
    lib/envelope.c 
    lib/codebook.c 
    lib/block.c 
    lib/bitrate.c 
    lib/analysis.c
	radxe/alloca.c
)
endif()


add_library(vorbis112 STATIC
	${SOURCES}
)


#install(TARGETS gsm DESTINATION lib)

