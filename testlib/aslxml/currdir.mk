#by jeff
LOCAL_DIR := $(call my-dir)
include $(CLEAR_VAR)

LOCAL_C_SRC  :=  \
         mxml-attr.c	\
         mxml-entity.c	\
         mxml-file.c	\
         mxml-get.c		\
         mxml-index.c	\
         mxml-node.c	\
         mxml-private.c \
         mxml-search.c	\
         mxml-set.c		\
         mxml-string.c	\
         mxmldoc.c


LOCAL_TARGET_SUFFIX				:= .a



