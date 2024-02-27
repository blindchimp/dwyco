add_compile_definitions(VCCFG_FILE DWYCO_APP_NICENAME="Phoo")

    add_compile_definitions(LINUX)
# Determine the host platform
if(UNIX)
    add_compile_definitions(LINUX)
	add_compile_options(-Wall -Wno-unused-parameter -Wno-reorder -Wno-unused-variable -Wno-unused-function)
endif()
if(WIN32)
    # Set compiler definitions for Windows
    add_compile_definitions(PLATFORM_WINDOWS)
endif()
if(APPLE)
message("FUCK")
    # Set compiler definitions for macOS
    add_compile_definitions(MACOSX)
endif()


