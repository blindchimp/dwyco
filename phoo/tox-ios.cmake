# Build c-toxcore and libsodium from source for iOS.
#
# c-toxcore's own CMake build normally discovers libsodium with pkg-config.
# During an iOS build that finds the macOS libsodium in /opt/homebrew, which
# cannot be linked into an iOS app. Instead, build libsodium for the same
# toolchain here and provide the libsodium::libsodium target that c-toxcore
# looks for, so its dependency logic uses our iOS build.

get_filename_component(DWYCO_TOXCORE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../../c-toxcore" ABSOLUTE)
if(NOT EXISTS "${DWYCO_TOXCORE_SRC}/CMakeLists.txt")
    message(FATAL_ERROR "c-toxcore not found at ${DWYCO_TOXCORE_SRC}")
endif()

# --- libsodium -------------------------------------------------------------
# Point DWYCO_LIBSODIUM_SRC at an unpacked libsodium source tree to avoid the
# download, or leave it empty to fetch the released tarball.
set(DWYCO_LIBSODIUM_SRC "" CACHE PATH
    "Path to an unpacked libsodium source tree to build for iOS (empty = download)")

if(DWYCO_LIBSODIUM_SRC)
    set(_libsodium_dir "${DWYCO_LIBSODIUM_SRC}")
else()
    include(FetchContent)
    set(_libsodium_fetch_args
        URL https://github.com/jedisct1/libsodium/releases/download/1.0.22-RELEASE/libsodium-1.0.22.tar.gz
        URL_HASH SHA256=adbdd8f16149e81ac6078a03aca6fc03b592b89ef7b5ed83841c086191be3349)
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.24")
        list(APPEND _libsodium_fetch_args DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    endif()
    FetchContent_Declare(libsodium ${_libsodium_fetch_args})
    FetchContent_MakeAvailable(libsodium)
    set(_libsodium_dir "${libsodium_SOURCE_DIR}")
endif()

set(DWYCO_SODIUM_SRC "${_libsodium_dir}/src/libsodium")
if(NOT EXISTS "${DWYCO_SODIUM_SRC}/include/sodium.h")
    message(FATAL_ERROR "libsodium source not found at ${DWYCO_SODIUM_SRC}")
endif()

# The release tarball does not contain the generated version.h, so create it
# the same way libsodium's configure script does.  Pull the numbers from
# configure.ac so an override source tree of a different version still works.
if(EXISTS "${_libsodium_dir}/configure.ac")
    file(STRINGS "${_libsodium_dir}/configure.ac" _sodium_ac_init
        REGEX "^AC_INIT\\(\\[libsodium\\]")
    string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" SODIUM_VERSION "${_sodium_ac_init}")

    file(STRINGS "${_libsodium_dir}/configure.ac" _sodium_line
        REGEX "^SODIUM_LIBRARY_VERSION_MAJOR=")
    string(REGEX REPLACE "^SODIUM_LIBRARY_VERSION_MAJOR=" ""
        SODIUM_LIBRARY_VERSION_MAJOR "${_sodium_line}")

    file(STRINGS "${_libsodium_dir}/configure.ac" _sodium_line
        REGEX "^SODIUM_LIBRARY_VERSION_MINOR=")
    string(REGEX REPLACE "^SODIUM_LIBRARY_VERSION_MINOR=" ""
        SODIUM_LIBRARY_VERSION_MINOR "${_sodium_line}")
endif()
if(NOT SODIUM_VERSION)
    set(SODIUM_VERSION "1.0.22")
    set(SODIUM_LIBRARY_VERSION_MAJOR 26)
    set(SODIUM_LIBRARY_VERSION_MINOR 4)
endif()
set(SODIUM_LIBRARY_MINIMAL_DEF "")
set(DWYCO_SODIUM_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/libsodium-generated")
file(MAKE_DIRECTORY "${DWYCO_SODIUM_GENERATED_DIR}/sodium")
file(READ "${DWYCO_SODIUM_SRC}/include/sodium/version.h.in" _sodium_version_h)
string(REPLACE "@VERSION@" "${SODIUM_VERSION}" _sodium_version_h "${_sodium_version_h}")
string(REPLACE "@SODIUM_LIBRARY_VERSION_MAJOR@" "${SODIUM_LIBRARY_VERSION_MAJOR}"
    _sodium_version_h "${_sodium_version_h}")
string(REPLACE "@SODIUM_LIBRARY_VERSION_MINOR@" "${SODIUM_LIBRARY_VERSION_MINOR}"
    _sodium_version_h "${_sodium_version_h}")
string(REPLACE "@SODIUM_LIBRARY_MINIMAL_DEF@" "${SODIUM_LIBRARY_MINIMAL_DEF}"
    _sodium_version_h "${_sodium_version_h}")
file(WRITE "${DWYCO_SODIUM_GENERATED_DIR}/sodium/version.h" "${_sodium_version_h}")
# version.h includes export.h with quotes, so keep a copy next to it.
configure_file(
    "${DWYCO_SODIUM_SRC}/include/sodium/export.h"
    "${DWYCO_SODIUM_GENERATED_DIR}/sodium/export.h"
    COPYONLY)

# libsodium normally gets these from its autotools configure run. Compiling the
# sources directly with CMake works fine as long as we supply the platform
# defines it expects. Note: iOS has no sys/random.h, so that one is left out.
file(GLOB_RECURSE DWYCO_SODIUM_SOURCES CONFIGURE_DEPENDS "${DWYCO_SODIUM_SRC}/*.c")
list(FILTER DWYCO_SODIUM_SOURCES EXCLUDE REGEX "/\\.deps/")
list(FILTER DWYCO_SODIUM_SOURCES EXCLUDE REGEX "/\\.libs/")

add_library(dwyco_sodium STATIC ${DWYCO_SODIUM_SOURCES})
target_include_directories(dwyco_sodium
    PUBLIC
        "${DWYCO_SODIUM_GENERATED_DIR}"
        "${DWYCO_SODIUM_SRC}/include"
    PRIVATE
        "${DWYCO_SODIUM_GENERATED_DIR}/sodium"
        "${DWYCO_SODIUM_SRC}/include/sodium")
target_compile_definitions(dwyco_sodium PRIVATE
    HAVE_ATOMIC_OPS=1
    HAVE_C11_MEMORY_FENCES=1
    HAVE_GCC_MEMORY_FENCES=1
    HAVE_C_VARRAYS=1
    HAVE_ALLOCA=1
    HAVE_ALLOCA_H=1
    HAVE_ARC4RANDOM=1
    HAVE_ARC4RANDOM_BUF=1
    HAVE_SYS_MMAN_H=1
    HAVE_SYS_PARAM_H=1
    HAVE_DLFCN_H=1
    HAVE_POSIX_MEMALIGN=1
    HAVE_MMAP=1
    HAVE_MLOCK=1
    HAVE_MADVISE=1
    HAVE_MPROTECT=1
    HAVE_NANOSLEEP=1
    HAVE_CLOCK_GETTIME=1
    HAVE_GETPID=1
    HAVE_SYSCONF=1
    HAVE_RAISE=1
    HAVE_MEMSET_S=1
    NATIVE_LITTLE_ENDIAN=1)
set_target_properties(dwyco_sodium PROPERTIES OUTPUT_NAME sodium)

# c-toxcore prefers this target over anything pkg-config finds.
add_library(libsodium::libsodium ALIAS dwyco_sodium)

set(DWYCO_SODIUM_INCLUDE_DIR
    "${DWYCO_SODIUM_GENERATED_DIR}"
    "${DWYCO_SODIUM_SRC}/include")

# --- c-toxcore -------------------------------------------------------------
# phoo only needs the static toxcore library, not toxav, the daemons, or tests.
set(BUILD_TOXAV OFF)
set(MUST_BUILD_TOXAV OFF)
set(UNITTEST OFF)
set(AUTOTEST OFF)
set(BUILD_MISC_TESTS OFF)
set(BUILD_FUN_UTILS OFF)
set(BUILD_FUZZ_TESTS OFF)
set(DHT_BOOTSTRAP OFF)
set(BOOTSTRAP_DAEMON OFF)
set(ENABLE_SHARED OFF)
set(ENABLE_STATIC ON)
set(STRICT_ABI OFF)

add_subdirectory("${DWYCO_TOXCORE_SRC}" "${CMAKE_CURRENT_BINARY_DIR}/c-toxcore")
