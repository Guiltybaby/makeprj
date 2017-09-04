#test by jeff

LOCAL_DIR := $(call my-dir)
include $(CLEAR_VAR)
LOCAL_SRC_FILE := 		src/atask.c \
			src/aosmgr.c \
			src/asemaphore_compact.c \
			src/aheap.c \
			src/zshmtmpfs.c \
			src/ashm.c 
LOCAL_MODULE:=os
LOCAL_TARGET_SUFFIX:=.a
LOCAL_SHARED_LIBRARIES:=
LOCAL_EXPORT_INC_PATH:=$(LOCAL_DIR)/inc

include $(BUILD_MODULE)

