#by jeff
LOCAL_DIR := $(call my-dir)
include $(CLEAR_VAR)

LOCAL_SRC_FILE:=src/file1.c src/file3.c
LOCAL_BIN_TARGET:=
LOCAL_MODULE:=mod1
LOCAL_TARGET_SUFFIX:=.so
LOCAL_INC_PATH:=
LOCAL_EXPORT_INC_PATH:= $(LOCAL_DIR)
LOCAL_LINK_FLAGS:=
LOCAL_STATIC_LIBRARIES:= libmod2
include $(BUILD_MODULE)


