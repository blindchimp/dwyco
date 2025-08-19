cmake_minimum_required(VERSION 3.14)

project(cdc32 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../${DWYCO_CONFDIR}/conf.cmake)
include_directories(
    ${VCCFG_COMP}
    ${CMAKE_CURRENT_SOURCE_DIR}/../dwcls
    ${CMAKE_CURRENT_SOURCE_DIR}/../zlib
    ${CMAKE_CURRENT_SOURCE_DIR}/../crypto8
    ${CMAKE_CURRENT_SOURCE_DIR}/../kazlib
    ${CMAKE_CURRENT_SOURCE_DIR}/../jenkins
    ${CMAKE_CURRENT_SOURCE_DIR}/../uv/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../vc
    ${CMAKE_CURRENT_SOURCE_DIR}/../pbm
    ${CMAKE_CURRENT_SOURCE_DIR}/../pgm
    ${CMAKE_CURRENT_SOURCE_DIR}/../ppm
    ${CMAKE_CURRENT_SOURCE_DIR}/../gsm
    ${CMAKE_CURRENT_SOURCE_DIR}/../speex/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../theora.1.2.x/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../ogg/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../vorbis112/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../uv/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../miniupnp/miniupnp-master/miniupnpc
    .
)

if(UNIX)
    include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../v4lcap)
    include_directories(winemu)
    set(EXTRA_FILES winemu.cc linid.cpp)
    add_compile_definitions(UWB_SAMPLING  UWB_SAMPLE_RATE=44100)
endif()

if(MSVC)
    set(EXTRA_FILES uniq.cpp audwin.cc aqaud.cc)
    add_compile_definitions(
        # don't use the "internal" vidgrab stuff anymore, it must be
        # done externally in the vgqt capture driver
        #VIDGRAB_HACKS
        UWB_SAMPLING  UWB_SAMPLE_RATE=44100
    )
endif()

if(APPLE AND NOT UNIX)
    add_compile_definitions(UWB_SAMPLING  UWB_SAMPLE_RATE=44100)
endif()

if(${DWYCOBG} EQUAL 1)
    add_compile_definitions(DWYCO_NO_THEORA_CODEC DWYCO_NO_GSM DWYCO_NO_VORBIS DWYCO_NO_UPNP DWYCO_NO_VIDEO_FROM_PPM DWYCO_NO_VIDEO_MSGS
)
endif()

# Definitions
add_compile_definitions(
        VCCFG_FILE
	DWVEC_DOINIT
        DWYCO_USE_SQLITE
        DWYCO_USE_STATIC_SQLITE
        DWYCO_NO_VIDEO_FROM_PPM
        DWYCO_MUTEX_NAME="${DWYCO_APP}"
)

add_compile_definitions($<$<CONFIG:Debug>:DW_RTLOG>)
# i forgot, adding this tracing stuff confuses the shit
# out of the clangd reference calculations and stuff in the ide.
# only use this if you really need the api tracing entries.
#add_compile_definitions($<$<CONFIG:Debug>:DWYCO_TRACE>)
add_compile_definitions($<$<CONFIG:Debug>:DWYCO_DBG_CHECK_SQL>)

if(${DWYCO_APP} STREQUAL "rando")
    add_compile_definitions(DWYCO_NO_THEORA_CODEC DWYCO_NO_GSM DWYCO_NO_VORBIS DWYCO_NO_UPNP DWYCO_NO_VIDEO_FROM_PPM DWYCO_NO_VIDEO_MSGS)
else()
    add_compile_definitions(MINIUPNP_STATICLIB)
endif()

add_library(cdc32 OBJECT
	activeuid.cpp 
	backandroid.cpp 
	bgapp.cpp 
	ezset2.cpp 
	mmchan.cc 
	mmbld.cc 
	mmaud.cc 
	mmband.cc 
	mmchan_sync.cpp 
	mmconn.cc 
	mmctrl.cc 
	mmchan2.cc 
	mmsync.cc 
	audconv.cc 
	audchk.cc 
	acqfile.cc 
	dct.cc 
	audmixs.cc 
	audout.cc 
	bcastq.cc 
	chroma.cc 
	codec.cc 
	colcod.cc 
	coldec.cc 
	dchroma.cc 
	profiledb.cpp 
	pulls.cpp 
	qtab.cc 
	dwrate.cc 
	dwrtlog.cc 
	dwtimer.cc 
	enc.cc 
	filetube.cc 
	gsmconv.cc 
	imgmisc.cc 
	jccolor.cc 
	jchuff.cc 
	jcparam.cc 
	jdcolor.cc 
	jdhuff.cc 
	jdmaster.cc 
	jfdctfst.cc 
	jfdctint.cc 
	jqtab.cc 
	jtcode.cc 
	jtdecode.cc 
	jutils.cc 
	mo.cc 
	netvid.cc 
	packbits.cc 
	qdirth.cc 
	qpol.cc 
	dwlog.cc 
	sync_sendq.cpp 
	synccalls.cpp 
	syncvar.cc 
	doinit.cc 
	netcod.cc 
	netcod2.cc 
	tcode.cc 
	statfun.cc 
	dirth.cc 
	rlc.cc 
	tdecode.cc 
	tpgmdec.cc 
	sqrs.cc 
	qauth.cc 
	senc.cc 
	aq.cc 
	vcpan.cc 
	ta.cc 
	audi.cc 
	audo.cc 
	aconn.cc 
	tpgmcod.cc 
	globs.cc 
	fl.cc 
	mmserv.cc 
	tl.cc 
	dlli.cpp 
	mcc.cpp 
	qmsg.cc 
	audexto.cc 
	aqext.cc 
	aqextaud.cc 
	audth.cc 
	audmix.cc 
	autoup.cpp 
	dmdsrv.cc 
	fnmod.cc 
	xinfo.cpp 
	vidcvt.cc 
	callq.cpp 
	asshole.cpp 
	servass.cpp 
	chatops.cc 
	mmcall.cpp 
	calllive.cpp 
	spxconv.cc 
	calldll.cpp 
	msgddll.cpp 
	jcolcod.cc 
	jcoldec.cc 
	netdiag.cpp 
	pgdll.cpp 
	chatgrid.cpp 
	chatq.cpp 
	sysattr.cpp 
	vorbconv.cc 
	se.cpp 
	theoracol.cc 
	ser.cpp 
	mmchan3.cpp 
	sproto.cpp 
	dhsetup.cpp 
	trc.cpp 
	cdcpal2.cc 
	ssns.cpp 
	qsend.cpp 
	directsend.cpp 
	msend.cpp 
	qmsgsql.cpp 
	fetch_to_inbox.cpp 
	sqlbq.cpp 
	aqext_android.cpp 
	dhgsetup.cpp 
	simplesql.cpp 
	grpmsg.cpp 
	upnp.cpp 
	aqkey.cpp 
	netlog.cpp
        sqlite3.c
        ${EXTRA_FILES}
)

target_compile_definitions(cdc32 PRIVATE CDC32_LIBRARY)
# anything over -O1 causes the xcode 16.3 compiler to generate
# code for the old DCT decoder that is broken. it might be
# something in the old code, but i couldn't easily find the
# problem. NOTE: this appears to be fixed by removing inlining
# in packbits.* note: -fno-strict-aliasing did NOT fix the problem,
# so inlining is the main suspect.
if(APPLE)
#target_compile_options(cdc32 PRIVATE $<$<CONFIG:Release>:-O3 -fno-strict-aliasing>)
endif()
