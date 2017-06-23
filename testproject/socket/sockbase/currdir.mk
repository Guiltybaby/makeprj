#test by jeff

LOCAL_DIR := $(call my-dir)
include $(CLEAR_VAR)
LOCAL_SRC_FILE := src/base.c
LOCAL_MODULE:=sockbase
LOCAL_TARGET_SUFFIX:=.a
LOCAL_STATIC_LIBRARIES:= libos libos
LOCAL_EXPORT_INC_PATH:=$(LOCAL_DIR)/inc

include $(BUILD_MODULE)

