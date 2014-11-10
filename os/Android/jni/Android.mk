#
#Calipso Jni module
#
LOCAL_PATH := $(call my-dir)
SRC_DIR :=../../../src
SRC_INC :=../../../src/include
MOD_DIR :=../$(SRC_DIR)/modules
VERSION := $(shell cat ../../../VERSION)

include $(CLEAR_VARS)
#include $(call all-subdir-makefiles)
LOCAL_CFLAGS := -Wall -g -DDEBUG -D_FILE_OFFSET_BITS=64 -D_REENTRANT -DVERSION="\"$(VERSION)\""
LOCAL_LDLIBS :=  -llog 
LOCAL_C_INCLUDES := $(SRC_INC)
LOCAL_MODULE := calipso-jni
LOCAL_SRC_FILES := calipso-jni.cpp \
	compat.c\
 	$(SRC_DIR)/calipso.c\
	$(SRC_DIR)/array.c\
	$(SRC_DIR)/list.c\
	$(SRC_DIR)/btree.c\
	$(SRC_DIR)/rbtree.c\
	$(SRC_DIR)/queue.c\
	$(SRC_DIR)/hash.c\
	$(SRC_DIR)/dllist.c\
	$(SRC_DIR)/http.c\
	$(SRC_DIR)/config.c\
	$(SRC_DIR)/socket.c\
	$(SRC_DIR)/signal.c\
	$(SRC_DIR)/cpo_io.c\
	$(SRC_DIR)/server.c\
	$(SRC_DIR)/client.c\
	$(SRC_DIR)/reply.c\
	$(SRC_DIR)/request.c\
	$(SRC_DIR)/resource.c\
	$(SRC_DIR)/event.c\
	$(SRC_DIR)/mpool.c\
	$(SRC_DIR)/process.c\
	$(SRC_DIR)/mprocess.c\
	$(SRC_DIR)/module.c\
	$(SRC_DIR)/hooks.c\
	$(SRC_DIR)/cplib/thread.c\
	$(SRC_DIR)/cplib/cplib.c\
	$(SRC_DIR)/cplib/xmalloc.c\
	$(SRC_DIR)/cplib/cp_time.c\
	$(SRC_DIR)/cplib/cpo_string.c\
	$(SRC_DIR)/cplib/cpo_file.c\
	$(SRC_DIR)/cplib/base64.c\
	$(SRC_DIR)/chunks.c\
	$(SRC_DIR)/cpo_log.c\
	$(SRC_DIR)/poll.c\
	$(SRC_DIR)/epoll.c\
	$(SRC_DIR)/timer.c

include $(BUILD_SHARED_LIBRARY)
LOCAL_SHARED_LIBRARIES := libc librt libpthread 

# create a temp variable with the current path, because it 
# changes after each include
#ZPATH := $(LOCAL_PATH)
#include $(ZPATH)/mod_http/Android.mk
#include $(ZPATH)/mod_autoind/Android.mk

#LOCAL_PATH:= $(call my-dir)
#include $(CLEAR_VARS)
#LOCAL_SRC_FILES := main.c
#LOCAL_SHARED_LIBRARIES := libc libhello
#LOCAL_MODULE := myprog
#include $(BUILD_EXECUTABLE)