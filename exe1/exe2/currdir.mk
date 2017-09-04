#by jeff
LOCAL_DIR := $(call my-dir)
include $(CLEAR_VAR)

LOCAL_SRC_FILE:=main.c cmm.c
LOCAL_MODULE:=m
LOCAL_TARGET_SUFFIX:=.exe
LOCAL_STATIC_LIBRARIES:= libmalloc_test
LOCAL_SHARED_LIBRARIES:= libmod1 
LOCAL_LD_FLAGS:= -Wl,--gc-sections -Wl,--wrap=malloc -Wl,--wrap=free #-Wl, --wrap calloc -Wl, --wrap realloc 

include $(BUILD_MODULE)

