# Standard things
LOCAL_DIR := $(call my-dir)
include $(CLEAR_VAR)


LOCAL_SRC_FILE:=file1.c
LOCAL_MODULE:=mod2
LOCAL_TARGET_SUFFIX:=.a
LOCAL_SHARED_LIBRARIES:= libmod1 
LOCAL_EXPORT_INC_PATH:=$(LOCAL_DIR)

include $(BUILD_MODULE)

