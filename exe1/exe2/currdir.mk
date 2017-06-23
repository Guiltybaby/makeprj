#by jeff
LOCAL_DIR := $(call my-dir)
include $(CLEAR_VAR)

LOCAL_SRC_FILE:=main.c cmm.c
LOCAL_MODULE:=m
LOCAL_LD_FLAGS:= -Wl,--gc-sections
LOCAL_TARGET_SUFFIX:=.exe
LOCAL_STATIC_LIBRARIES:= libmod2
LOCAL_SHARED_LIBRARIES:= libmod1 

include $(BUILD_MODULE)

