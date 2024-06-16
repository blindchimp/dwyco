
cmake_minimum_required(VERSION 3.14)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../${DWYCO_CONFDIR}/conf.cmake)

project(theora.1.2.x LANGUAGES C)
include_directories(include ../ogg/include ../vorbis/include)


set(SOURCES
    lib/tokenize.c 
    lib/state.c 
    lib/rate.c 
    lib/quant.c 
    lib/mcenc.c 
    lib/mathops.c 
    lib/internal.c 
    lib/info.c 
    lib/idct.c 
    lib/huffenc.c 
    lib/huffdec.c 
    lib/fragment.c 
    lib/fdct.c 
    lib/enquant.c 
    lib/encode.c 
    lib/encinfo.c 
    lib/encfrag.c 
    lib/dequant.c 
    lib/decode.c 
    lib/decinfo.c 
    lib/bitpack.c 
    lib/apiwrapper.c 
    lib/analyze.c 
    lib/encapiwrapper.c 
    lib/decapiwrapper.c
)

if(UNIX)
add_compile_definitions(OC_X86_ASM OC_X86_64_ASM)
list(APPEND SOURCES
        lib/x86/x86cpu.c 
        lib/x86/mmxencfrag.c 
        lib/x86/mmxfdct.c 
        lib/x86/x86enc.c 
        lib/x86/x86enquant.c 
        lib/x86/sse2fdct.c 
        lib/x86/mmxfrag.c 
        lib/x86/mmxidct.c 
        lib/x86/mmxstate.c 
        lib/x86/x86state.c 
        lib/x86/sse2encfrag.c 
        lib/x86/sse2idct.c
)
endif()

if(WIN32)
add_compile_definitions(OC_X86_ASM)
list(APPEND SOURCES
        lib/x86_vc/x86cpu.c 
        lib/x86_vc/mmxencfrag.c 
        lib/x86_vc/mmxfdct.c 
        lib/x86_vc/x86enc.c 
        lib/x86_vc/mmxfrag.c 
        lib/x86_vc/mmxidct.c 
        lib/x86_vc/mmxstate.c 
        lib/x86_vc/x86state.c

)
endif()

add_library(theora.1.2.x STATIC
	${SOURCES}
)


#install(TARGETS gsm DESTINATION lib)

