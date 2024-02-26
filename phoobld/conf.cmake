add_compile_definitions(VCCFG_FILE DWYCO_APP_NICENAME="Phoo")

    add_compile_definitions(LINUX)
# Determine the host platform
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # Set compiler definitions for Linux
message("FUCK")
    add_compile_definitions(LINUX)
	add_compile_options(-Wall -Wno-unused-parameter -Wno-reorder -Wno-unused-variable -Wno-unused-function)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # Set compiler definitions for Windows
    add_compile_definitions(PLATFORM_WINDOWS)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # Set compiler definitions for macOS
    add_compile_definitions(PLATFORM_MACOS)
endif()


