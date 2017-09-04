#test by jeff

LOCAL_DIR := $(call my-dir)
include $(CLEAR_VAR)
LOCAL_SRC_FILE := src/main.c
LOCAL_MODULE:=shmclient
LOCAL_TARGET_SUFFIX:=.exe
LOCAL_STATIC_LIBRARIES:= libos

include $(BUILD_MODULE)

