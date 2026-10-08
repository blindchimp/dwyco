set(DWYCO_APP "dwytest")
set(DWYCOBG 1)
set(DWYCO_TOXCORE OFF)

# Testing config. Lets cdc32 keep the parts of the library that the tests
# actually need to exercise, which DWYCOBG=1 would otherwise strip out:
#
#   DWYCO_NO_THEORA_CODEC   video codec. Without it coder_from_config() has no
#                           coder to instantiate, so build_outgoing() fails and
#                           capture can never initialize at all.
#   DWYCO_NO_VIDEO_FROM_PPM the built-in file-based capture source, so video
#                           capture can be tested with no camera attached.
#   DWYCO_NO_VIDEO_MSGS     dwyco_zap_create_preview(), the PNG-writing variant
#                           of the zap preview api.
#
# Deliberately still stripped: DWYCO_NO_GSM, DWYCO_NO_VORBIS, DWYCO_NO_UPNP
# and DWYCO_NO_ACQ_VIDEO_MEDIA (the external camera-driver path, which the
# media-state test asserts is compiled out).
#
# See the DWYCO_TESTING branch in bld/cdc32/CMakeLists.txt for the other half.
set(DWYCO_TESTING 1)

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
