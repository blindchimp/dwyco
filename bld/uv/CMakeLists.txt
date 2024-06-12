
cmake_minimum_required(VERSION 3.14)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../${DWYCO_CONFDIR}/conf.cmake)

project(uv LANGUAGES C)
include_directories(src include include/uv-private)
if(UNIX)
add_compile_definitions(_LARGEFILE_SOURCE _FILE_OFFSET_BITS=64)
add_compile_definitions(_GNU_SOURCE _POSIX_C_SOURCE=200112)
if(ANDROID)
    add_compile_definitions(LINUX ANDROID)
endif()
set(SOURCES
        src/fs-poll.c 
        src/inet.c 
        src/uv-common.c 
        src/uv-common.h 
        src/version.c
            src/unix/async.c 
            src/unix/atomic-ops.h 
            src/unix/core.c 
            src/unix/dl.c 
            src/unix/error.c 
            src/unix/fs.c 
            src/unix/getaddrinfo.c 
            src/unix/internal.h 
            src/unix/loop.c 
            src/unix/loop-watcher.c 
            src/unix/pipe.c 
            src/unix/poll.c 
            src/unix/process.c 
            src/unix/signal.c 
            src/unix/stream.c 
            src/unix/tcp.c 
            src/unix/thread.c 
            src/unix/threadpool.c 
            src/unix/timer.c 
            src/unix/tty.c 
            src/unix/udp.c 
            src/unix/proctitle.c
            src/unix/linux-core.c  
            src/unix/linux-inotify.c 
            src/unix/linux-syscalls.c 
            src/unix/linux-syscalls.h
)
endif()

if(APPLE)
add_compile_definitions(_DARWIN_USE_64_BIT_INODE=1)
set(SOURCES
        src/fs-poll.c 
        src/inet.c 
        src/uv-common.c 
        src/uv-common.h 
        src/version.c
            src/unix/async.c 
            src/unix/atomic-ops.h 
            src/unix/core.c 
            src/unix/dl.c 
            src/unix/error.c 
            src/unix/fs.c 
            src/unix/getaddrinfo.c 
            src/unix/internal.h 
            src/unix/loop.c 
            src/unix/loop-watcher.c 
            src/unix/pipe.c 
            src/unix/poll.c 
            src/unix/process.c 
            src/unix/signal.c 
            src/unix/stream.c 
            src/unix/tcp.c 
            src/unix/thread.c 
            src/unix/threadpool.c 
            src/unix/timer.c 
            src/unix/tty.c 
            src/unix/udp.c 
            src/unix/proctitle.c
            src/unix/darwin.c 
            src/unix/fsevents.c 
            src/unix/darwin-proctitle.c 
            src/unix/kqueue.c
)
endif()

if(WIN32)
add_compile_definitions(_WIN32_WINNT=0x0600)
set(SOURCES
        src/fs-poll.c 
        src/inet.c 
        src/uv-common.c 
        src/uv-common.h 
        src/version.c
            src/win/async.c 
            src/win/core.c 
            src/win/dl.c 
            src/win/error.c 
            src/win/fs.c 
            src/win/fs-event.c 
            src/win/getaddrinfo.c 
            src/win/handle.c 
            src/win/loop-watcher.c 
            src/win/pipe.c 
            src/win/thread.c 
            src/win/poll.c 
            src/win/process.c 
            src/win/process-stdio.c 
            src/win/req.c 
            src/win/signal.c 
            src/win/stream.c 
            src/win/tcp.c 
            src/win/tty.c 
            src/win/threadpool.c 
            src/win/timer.c 
            src/win/udp.c 
            src/win/util.c 
            src/win/winapi.c 
            src/win/winsock.c
        )
endif()


add_library(uv STATIC
	${SOURCES}
)

target_compile_definitions(uv PUBLIC _GNU_SOURCE _POSIX_C_SOURCE=200112)
#install(TARGETS gsm DESTINATION lib)

