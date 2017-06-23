#test by jeff

LOCAL_DIR := $(call my-dir)
include $(CLEAR_VAR)
LOCAL_SRC_FILE := src/main.c src/mem.c 
LOCAL_MODULE:=mtest
LOCAL_TARGET_SUFFIX:=.exe
LOCAL_LINK_FLAGS+= --wrap=malloc
#LOCAL_SHARED_LIBRARIES:= libos

include $(BUILD_MODULE)


