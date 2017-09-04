### Build flags for all targets
#
.SUFFIXES:

ifeq ($(QUIET),)
Q:=
endif

include build/definition.mk

BUILD_ROOT := $(call my-dir)
PRJ_ROOT := .

CLEAR_VAR:=$(BUILD_ROOT)/clear_local.mk
BUILD_MODULE:=$(BUILD_ROOT)/build_module.mk
TOOLCHAIN:=toolchain

### Build tools
# 

MULTI_ARCH:= 64 #32 


.PHONY:	all	targets

all: targets
		echo $<
		@echo "			********************************"
		@echo "			*                              *"
		@echo "			*                              *"
		@echo "			*                              *"
		@echo "			*          All Success         *"
		@echo "			*                              *"
		@echo "			*                              *"
		@echo "			*                              *"
		@echo "			********************************"

#TODO: auto search dir

include  $(foreach arch,$(MULTI_ARCH),build/tool_$(arch).mk build/dirgen_$(arch).mk $(PRJ_ROOT)/cfg/module.mk)

targets:	$(MODULE_ALL)

.PHONY:		clean
clean:
		@rm -f $(CLEAN)
		@echo "clean Success"



