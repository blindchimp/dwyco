TEMPLATE = lib
CONFIG += staticlib
CONFIG -= qt

include($$PWD/../../$$DWYCO_CONFDIR/conf.pri)

INCLUDEPATH +=  $${VCCFG_COMP} ../dwcls ../zlib ../crypto5 ../kazlib ../jenkins ../uv/include
DEFINES += VC_INTERNAL # DWYCO_NO_TSOCK

!win32 {
DEFINES += UNIX
QMAKE_CXXFLAGS += #-fpermissive
#QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-reorder
}

equals(LH_INTERPRETER, 1) {
# note: this simple lookup caching works, but is only really
# useful in situations where a program doesn't make a lot of
# function calls to user defined functions. in order to improve
# it would require something quite a bit more complicated, or changing
# some of the semantics of the language.
DEFINES += LHOBJ PERFHACKS FUNCACHE #CACHE_LOOKUPS
}
!equals(LH_INTERPRETER, 1) {
DEFINES += NO_VCEVAL
}

SOURCES = \
vclhnet.cpp \
dwgrows.cpp \
pmatch.cpp \
vc.cpp \
vc2.cpp \
vc3.cpp \
vcatomic.cpp \
vccfun.cpp \
vccomp.cpp \
vcctx.cpp \
vccvar.cpp \
vcdbg.cpp \
vcdbl.cpp \
vcdecom.cpp \
vcdeflt.cpp \
vcenco.cpp \
vcexcctx.cpp \
vcfunc.cpp \
vcfuncal.cpp \
vcfunctx.cpp \
vcfundef.cpp \
vcint.cpp \
vclex.cpp \
vclh.cpp \
vcmap.cpp \
vcnil.cpp \
vcregex.cpp \
vcser.cpp \
vcsock.cpp \
vcsrc.cpp \
vcstr.cpp \
vcwserr.cpp \
vcwsock.cpp \
vcxstrm.cpp \
vcmemfun.cpp \
vcmemsel.cpp \
vcobj.cpp \
vcfac.cpp \
vclhfac.cpp \
vcmath.cpp \
vclhsys.cpp \
vcfile.cpp \
vcsfile.cpp  \
enc.cpp \
vccrypt2.cpp \
zcomp.cpp \
vcio.cpp \
vcuvsock.cpp \
lhuvsock.cpp \
vctrans.cpp \
vcudh.cpp \
reobj.cpp \
vctsock.cpp
