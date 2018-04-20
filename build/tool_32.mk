#COMPILE_TOOLCHAIN		:=$(TOOLCHAIN)/linux-x86/arm/arm-linux-androideabi-4.9
#CROSS_COMPLIER  :=$(COMPILE_TOOLCHAIN)/bin/arm-linux-androideabi-
#REF_ANDROID_PATH:= /file/jamie_yuan/AndroidPlatform/mtk/android-5.1-tz/

#STDINC:= \
	$(REF_ANDROID_PATH)bionic/libc/arch-arm/include \
	$(REF_ANDROID_PATH)bionic/libc/include \
	$(REF_ANDROID_PATH)bionic/libstdc++/include \
	$(REF_ANDROID_PATH)bionic/libc/kernel/uapi \
	$(REF_ANDROID_PATH)bionic/libc/kernel/uapi/asm-arm \
	$(REF_ANDROID_PATH)bionic/libm/include \
	$(REF_ANDROID_PATH)bionic/libm/include/arm \
	$(REF_ANDROID_PATH)system/core/include \
	$(REF_ANDROID_PATH)frameworks/native/include 

#STDLIB:= \
	log c stdc++ m dl cutils
#STD_STATIC_LIB:= \
	$(REF_ANDROID_PATH)out/target/product/evb6795_64_tee_pr/obj_arm/STATIC_LIBRARIES/libcutils_intermediates/libcutils.a \
	$(REF_ANDROID_PATH)out/target/product/evb6795_64_tee_pr/obj_arm/STATIC_LIBRARIES/liblog_intermediates/liblog.a \
	$(REF_ANDROID_PATH)out/target/product/evb6795_64_tee_pr/obj_arm/STATIC_LIBRARIES/libc_intermediates/libc.a \
	$(COMPILE_TOOLCHAIN)/lib/gcc/arm-linux-androideabi/4.9.x-google/armv7-a/libgcc.a

#STD_DYNAMIC_LIB:= \
	$(REF_ANDROID_PATH)out/target/product/evb6795_64_tee_pr/obj_arm/lib/liblog.so \
	$(REF_ANDROID_PATH)out/target/product/evb6795_64_tee_pr/obj_arm/lib/libc.so \
	$(REF_ANDROID_PATH)out/target/product/evb6795_64_tee_pr/obj_arm/lib/libstdc++.so \
	$(REF_ANDROID_PATH)out/target/product/evb6795_64_tee_pr/obj_arm/lib/libm.so \
	$(REF_ANDROID_PATH)out/target/product/evb6795_64_tee_pr/obj_arm/lib/libdl.so \
	$(REF_ANDROID_PATH)out/target/product/evb6795_64_tee_pr/obj_arm/lib/libcutils.so

#CRT_DYNAMIC:= $(REF_ANDROID_PATH)/out/target/product/evb6795_64_tee_pr/obj_arm/lib/crtbegin_dynamic.o
#CRT_STATIC:= $(REF_ANDROID_PATH)/out/target/product/evb6795_64_tee_pr/obj_arm/lib/crtbegin_static.o
#CRT_END:=$(REF_ANDROID_PATH)out/target/product/evb6795_64_tee_pr/obj_arm/lib/crtend_android.o


ARCH			:= 32
CC              := $(CROSS_COMPLIER)gcc -fPIC
LD				:= $(CROSS_COMPLIER)g++
AR				:= $(CROSS_COMPLIER)ar
STD_INC			:= 
C_FLAGS         := -g -Wall -fPIC -I./include
ARCH_LD_FLAGS   := 
LF_ALL          := 
LL_ALL          := cr 

