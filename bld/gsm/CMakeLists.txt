
cmake_minimum_required(VERSION 3.14)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../${DWYCO_CONFDIR}/conf.cmake)

project(gsm LANGUAGES C)
add_compile_definitions(NDEBUG FAST SASR NeedFunctionPrototypes=1 )

add_library(gsm STATIC
add.c 
code.c 
debug.c 
decode.c 
gsm_cr~1.c 
gsm_de~1.c 
gsm_de~2.c 
gsm_en~1.c 
gsm_ex~1.c 
gsm_im~1.c 
gsm_op~1.c 
gsm_pr~1.c 
long_t~1.c 
lpc.c 
prepro~1.c 
rpe.c 
short_~1.c 
table.c
)

#install(TARGETS gsm DESTINATION lib)

