
TEMPLATE=lib
CONFIG -= qt
CONFIG += static

INCLUDEPATH += src include include/uv-private

!win32 {
DEFINES += _LARGEFILE_SOURCE _FILE_OFFSET_BITS=64
}

macx-clang|macx-ios-clang {
DEFINES += _DARWIN_USE_64_BIT_INODE=1
}

linux-* {
DEFINES += _GNU_SOURCE _POSIX_C_SOURCE=200112
}

win32 {
DEFINES += _WIN32_WINNT=0x0600
}

SOURCES = \
        src/fs-poll.c \
        src/inet.c \
        src/uv-common.c \
        src/uv-common.h \
        src/version.c

!win32 {
SOURCES += \
            src/unix/async.c \
            src/unix/atomic-ops.h \
            src/unix/core.c \
            src/unix/dl.c \
            src/unix/error.c \
            src/unix/fs.c \
            src/unix/getaddrinfo.c \
            src/unix/internal.h \
            src/unix/loop.c \
            src/unix/loop-watcher.c \
            src/unix/pipe.c \
            src/unix/poll.c \
            src/unix/process.c \
            src/unix/signal.c \
            src/unix/stream.c \
            src/unix/tcp.c \
            src/unix/thread.c \
            src/unix/threadpool.c \
            src/unix/timer.c \
            src/unix/tty.c \
            src/unix/udp.c \
            src/unix/proctitle.c
}

win32 {
SOURCES += \
            src/win/async.c \
            src/win/core.c \
            src/win/dl.c \
            src/win/error.c \
            src/win/fs.c \
            src/win/fs-event.c \
            src/win/getaddrinfo.c \
            src/win/handle.c \
            src/win/loop-watcher.c \
            src/win/pipe.c \
            src/win/thread.c \
            src/win/poll.c \
            src/win/process.c \
            src/win/process-stdio.c \
            src/win/req.c \
            src/win/signal.c \
            src/win/stream.c \
            src/win/tcp.c \
            src/win/tty.c \
            src/win/threadpool.c \
            src/win/timer.c \
            src/win/udp.c \
            src/win/util.c \
            src/win/winapi.c \
            src/win/winsock.c
}

linux {
SOURCES += \
            src/unix/linux-core.c \ 
            src/unix/linux-inotify.c \
            src/unix/linux-syscalls.c \
            src/unix/linux-syscalls.h
}

macx-clang|macx-ios-clang {
SOURCES += \
            src/unix/darwin.c \
            src/unix/fsevents.c \
            src/unix/darwin-proctitle.c \
            src/unix/kqueue.c
}
