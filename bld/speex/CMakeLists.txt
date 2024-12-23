
cmake_minimum_required(VERSION 3.14)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../${DWYCO_CONFDIR}/conf.cmake)

project(speex LANGUAGES C)
include_directories(include lib ../ogg/include)
if(WIN32)
include_directories(win32/rad2007)
endif()


add_compile_definitions(EXPORT= USE_KISS_FFT)
if(ANDROID)
add_compile_definitions(FIXED_POINT)
else()
add_compile_definitions(FLOATING_POINT)
endif()
if(WIN32)
add_compile_definitions(_USE_MATH_DEFINES)
endif()

set(SOURCES libspeex/resample.c)

add_library(speex OBJECT
	${SOURCES}
)


#install(TARGETS gsm DESTINATION lib)

