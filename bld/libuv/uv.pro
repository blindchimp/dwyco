
TEMPLATE=lib
CONFIG -= qt
CONFIG += static

INCLUDEPATH += src include include/uv-private

!win32 {
DEFINES += _LARGEFILE_SOURCE _FILE_OFFSET_BITS=64
}

macx-clang {
DEFINES += _DARWIN_USE_64_BIT_INODE=1
}

linux-* {
DEFINES += _GNU_SOURCE _POSIX_C_SOURCE=200112
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

macx-clang {
SOURCES += \
            src/unix/darwin.c \
            src/unix/fsevents.c \
            src/unix/darwin-proctitle.c
}
