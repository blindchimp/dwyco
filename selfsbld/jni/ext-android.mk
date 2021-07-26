LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

OGG_LIBS := $(addprefix libogg/output/lib/, \
 libogg.a )

VORBIS_LIBS := $(addprefix libvorbis/output/lib/, \
 libvorbis.a \
 libvorbisenc.a \
 libvorbisfile.a )

THEORA_LIBS := $(addprefix libtheora/output/lib/, \
 libtheora.a \
 libtheoraenc.a \
 libtheoradec.a )

SPEEX_LIBS := $(addprefix libspeex/output/lib/, \
 libspeex.a \
 libspeexdsp.a )
