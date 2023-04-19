LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := uv

# NOTE: self-contained, so don't import any project-level
# defines or config
LOCAL_C_FLAGS := -DANDROID -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 \
	-D_GNU_SOURCE -D_POSIX_C_SOURCE=200112


LOCAL_SRC_FILES = \
src/fs-poll.c \
src/inet.c \
src/uv-common.c \
src/version.c \
src/unix/async.c \
src/unix/core.c \
src/unix/dl.c \
src/unix/error.c \
src/unix/fs.c \
src/unix/getaddrinfo.c \
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
src/unix/proctitle.c \
src/unix/linux-core.c \
src/unix/linux-inotify.c \
src/unix/linux-syscalls.c

LOCAL_C_INCLUDES := \
$(LOCAL_PATH)/src \
$(LOCAL_PATH)/include \
$(LOCAL_PATH)/include/uv-private


include $(BUILD_STATIC_LIBRARY)

