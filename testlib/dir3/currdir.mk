#by jeff
LOCAL_DIR := $(call my-dir)
include $(CLEAR_VAR)


LOCAL_SRC_FILE:=file1.c file3.c
LOCAL_ASM_SRC:=
LOCAL_BIN_TARGET:=
LOCAL_MODULE:=mod3
LOCAL_TARGET_SUFFIX:=.so
LOCAL_INC_PATH:=
LOCAL_LINK_FLAGS:=
LOCAL_STATIC_LIBRARIES:= 
LOCAL_SHARED_LIBRARIES:= 

include $(BUILD_MODULE)


