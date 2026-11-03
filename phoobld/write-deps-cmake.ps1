param(
    [string]$ToxcoreRoot
)

# Write a patched Dependencies.cmake that doesn't require pkg-config on MSVC.
@"
###############################################################################
#
# :: For systems that have pkg-config.
#
###############################################################################

if(MSVC)
  find_package(PkgConfig QUIET)
else()
  find_package(PkgConfig REQUIRED)
endif()

find_library(NSL_LIBRARIES    nsl   )
find_library(RT_LIBRARIES     rt    )
find_library(SOCKET_LIBRARIES socket)

find_package(pthreads QUIET)
if(NOT TARGET PThreads4W::PThreads4W)
  find_package(pthreads4w QUIET)
endif()
if(NOT TARGET pthreads4w::pthreads4w)
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)
endif()

# For toxcore.
if(PKG_CONFIG_FOUND)
  pkg_search_module(LIBSODIUM   libsodium IMPORTED_TARGET)
endif()
if(MSVC)
  find_package(libsodium QUIET)
  if(NOT TARGET libsodium::libsodium)
    find_package(unofficial-sodium QUIET)
  endif()
endif()

# For toxav.
if(PKG_CONFIG_FOUND)
  pkg_search_module(OPUS        opus      IMPORTED_TARGET)
  if(NOT OPUS_FOUND)
    pkg_search_module(OPUS      Opus      IMPORTED_TARGET)
  endif()
endif()
if(NOT OPUS_FOUND)
  find_package(Opus QUIET)
  if(TARGET Opus::opus)
    set(OPUS_FOUND TRUE)
  endif()
endif()

if(PKG_CONFIG_FOUND)
  pkg_search_module(VPX         vpx       IMPORTED_TARGET)
  if(NOT VPX_FOUND)
    pkg_search_module(VPX       libvpx    IMPORTED_TARGET)
  endif()
endif()
if(NOT VPX_FOUND)
  find_package(libvpx QUIET)
  if(TARGET libvpx::libvpx)
    set(VPX_FOUND TRUE)
  endif()
endif()

# For tox-bootstrapd.
if(PKG_CONFIG_FOUND)
  pkg_search_module(LIBCONFIG   libconfig IMPORTED_TARGET)
endif()
"@ | Set-Content -Path "$ToxcoreRoot\cmake\Dependencies.cmake" -NoNewline

Write-Host "  Patched $ToxcoreRoot\cmake\Dependencies.cmake"
