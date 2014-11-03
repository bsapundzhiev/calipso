LOCAL_PATH := $(call my-dir)
MOD_NAME := mod_autoind
include $(CLEAR_VARS)
LOCAL_SRC_FILES := $(MOD_DIR)/$(MOD_NAME)/mod_autoind.c
LOCAL_CFLAGS := -fPIC -Wall -g -D_MODULE -DDEBUG -D_FILE_OFFSET_BITS=64 -D_REENTRANT
LOCAL_LDFLAGS := -shared
LOCAL_C_INCLUDES := $(APP_PROJECT_PATH) $(SRC_INC) $(MOD_DIR)/$(MOD_NAME) 
LOCAL_SHARED_LIBRARIES := libcalipso-jni
LOCAL_MODULE := $(MOD_NAME)
include $(BUILD_SHARED_LIBRARY)
