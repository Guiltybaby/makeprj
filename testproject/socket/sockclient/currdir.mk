#test by jeff

LOCAL_DIR := $(call my-dir)
include $(CLEAR_VAR)
LOCAL_SRC_FILE := src/main.c
LOCAL_MODULE:=sockclient
LOCAL_TARGET_SUFFIX:=.exe
LOCAL_STATIC_LIBRARIES:= libsockbase libos

include $(BUILD_MODULE)

