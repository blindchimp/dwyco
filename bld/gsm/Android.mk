LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE=gsm

#
# list of component level C and C++ options
#
LOCAL_CFLAGS := $(MY_CONF_LOCAL_CFLAGS) -DNDEBUG -DFAST -DSASR -DNeedFunctionPrototypes=1 

LOCAL_SRC_FILES=   \
add.c \
code.c \
debug.c \
decode.c \
gsm_cr~1.c \
gsm_de~1.c \
gsm_de~2.c \
gsm_en~1.c \
gsm_ex~1.c \
gsm_im~1.c \
gsm_op~1.c \
gsm_pr~1.c \
long_t~1.c \
lpc.c \
prepro~1.c \
rpe.c \
short_~1.c \
table.c

include $(BUILD_STATIC_LIBRARY)
