#test by jeff

LOCAL_DIR := $(call my-dir)
include $(CLEAR_VAR)
LOCAL_SRC_FILE := src/main.cpp src/time.cpp
LOCAL_MODULE:=cpptest
LOCAL_TARGET_SUFFIX:=.exe

include $(BUILD_MODULE)


