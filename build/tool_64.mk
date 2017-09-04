#COMPILE_TOOLCHAIN		:=$(TOOLCHAIN)/linux-x86/aarch64/aarch64-linux-android-4.9/
#CROSS_COMPLIER  :=$(COMPILE_TOOLCHAIN)bin/aarch64-linux-android-
#REF_ANDROID_PATH:= /file/jamie_yuan/AndroidPlatform/mtk/android-5.1-tz/

#STDINC:= \
-isystem $(REF_ANDROID_PATH)system/core/include \
-isystem $(REF_ANDROID_PATH)bionic/libc/arch-arm64/include \
-isystem $(REF_ANDROID_PATH)bionic/libc/include \
-isystem $(REF_ANDROID_PATH)bionic/libstdc++/include \
-isystem $(REF_ANDROID_PATH)bionic/libc/kernel/uapi \
-isystem $(REF_ANDROID_PATH)bionic/libc/kernel/uapi/asm-arm64 \
-isystem $(REF_ANDROID_PATH)bionic/libm/include \
-isystem $(REF_ANDROID_PATH)bionic/libm/include/arm64 \
-isystem $(REF_ANDROID_PATH)build/core/combo/include/arch/linux-arm64 \
-isystem $(REF_ANDROID_PATH)frameworks/native/include 

#STD_STATIC_LIB:= \
	$(REF_ANDROID_PATH)out/target/product/evb6795_64_tee_pr/obj/STATIC_LIBRARIES/libcutils_intermediates/libcutils.a \
	$(REF_ANDROID_PATH)out/target/product/evb6795_64_tee_pr/obj/STATIC_LIBRARIES/liblog_intermediates/liblog.a \
	$(REF_ANDROID_PATH)out/target/product/evb6795_64_tee_pr/obj/STATIC_LIBRARIES/libc_intermediates/libc.a \
	$(COMPILE_TOOLCHAIN)/lib/gcc/aarch64-linux-android/4.9.x-google/libgcc.a

#STD_DYNAMIC_LIB:= \
	$(REF_ANDROID_PATH)out/target/product/evb6795_64_tee_pr/obj/lib/liblog.so \
	$(REF_ANDROID_PATH)out/target/product/evb6795_64_tee_pr/obj/lib/libc.so \
	$(REF_ANDROID_PATH)out/target/product/evb6795_64_tee_pr/obj/lib/libstdc++.so \
	$(REF_ANDROID_PATH)out/target/product/evb6795_64_tee_pr/obj/lib/libm.so \
	$(REF_ANDROID_PATH)out/target/product/evb6795_64_tee_pr/obj/lib/libdl.so \
	$(REF_ANDROID_PATH)out/target/product/evb6795_64_tee_pr/obj/lib/libcutils.so

#CRT_DYNAMIC:= $(REF_ANDROID_PATH)/out/target/product/evb6795_64_tee_pr/obj/lib/crtbegin_dynamic.o
#CRT_STATIC:= $(REF_ANDROID_PATH)/out/target/product/evb6795_64_tee_pr/obj/lib/crtbegin_static1.o
#CRT_END:=$(REF_ANDROID_PATH)out/target/product/evb6795_64_tee_pr/obj/lib/crtend_android.o


ARCH			:= 64
CC              := $(CROSS_COMPLIER)gcc -fPIC
LD				:= $(CROSS_COMPLIER)g++
AR				:= $(CROSS_COMPLIER)ar
C_FLAGS         := -g -Wall -fPIC -I./include
ARCH_LD_FLAGS   := -lpthread
LL_ALL          := cr 

