set(DWYCO_APP "dwytest")
set(DWYCOBG 1)
set(DWYCO_TOXCORE OFF)

add_compile_definitions(
    CDCCORE_STATIC
    DWYCO_VC_CONV
    VCCFG_FILE
)

set(VCCFG_COMP ${CMAKE_CURRENT_SOURCE_DIR}/../../${DWYCO_CONFDIR})

if(UNIX AND NOT APPLE)
    add_compile_definitions(UNIX LINUX)
    add_compile_options(-Wall -Wno-unused-parameter -Wno-reorder -Wno-unused-variable -Wno-unused-function)
endif()
if(APPLE)
    add_compile_definitions(MACOSX LINUX UNIX)
    add_compile_options(-Wall -Wno-unused-parameter -Wno-reorder -Wno-unused-variable -Wno-unused-function)
endif()
