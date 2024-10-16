TARGET = speex
TEMPLATE = lib
CONFIG += static
CONFIG -= qt

include($$PWD/../../$$DWYCO_CONFDIR/conf.pri)

INCLUDEPATH += include ../ogg/include
win32: INCLUDEPATH += win32/rad2007
#DEFINES += HAVE_CONFIG_H
DEFINES += EXPORT= USE_KISS_FFT
android: DEFINES += FIXED_POINT
!android: DEFINES += FLOATING_POINT
win32: DEFINES += _USE_MATH_DEFINES

SOURCES += libspeex/resample.c

#SOURCES += \
#libspeex/bits.c \
#libspeex/buffer.c \
#libspeex/cb_search.c \
#libspeex/exc_10_16_table.c \
#libspeex/exc_10_32_table.c \
#libspeex/exc_20_32_table.c \
#libspeex/exc_5_256_table.c \
#libspeex/exc_5_64_table.c \
#libspeex/exc_8_128_table.c \
#libspeex/fftwrap.c \
#libspeex/filterbank.c \
#libspeex/filters.c \
#libspeex/gain_table.c \
#libspeex/gain_table_lbr.c \
#libspeex/hexc_10_32_table.c \
#libspeex/hexc_table.c \
#libspeex/high_lsp_tables.c \
#libspeex/jitter.c \
#libspeex/kiss_fft.c \
#libspeex/kiss_fftr.c \
#libspeex/lpc.c \
#libspeex/lsp.c \
#libspeex/lsp_tables_nb.c \
#libspeex/ltp.c \
#libspeex/mdf.c \
#libspeex/modes.c \
#libspeex/modes_wb.c \
#libspeex/nb_celp.c \
#libspeex/preprocess.c \
#libspeex/quant_lsp.c \
#libspeex/resample.c \
#libspeex/sb_celp.c \
#libspeex/scal.c \
#libspeex/smallft.c \
#libspeex/speex.c \
#libspeex/speex_callbacks.c \
#libspeex/speex_header.c \
#libspeex/stereo.c \
#libspeex/vbr.c \
#libspeex/vq.c \
#libspeex/window.c

HEADERS +=
