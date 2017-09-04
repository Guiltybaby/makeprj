#ifndef __ALOG_HEADER__
#define __ALOG_HEADER__

#include <stdio.h>
#include <aos.h>
#define A_LOGD(f,...) \
		printf("%-15s:%04d:pid=%d:ctid=%ld => "f"\n",__FUNCTION__, __LINE__,getpid(),gettid(),##__VA_ARGS__)

#define SL_LOGD(f,...) printf("%-15s:%04d:pid=%d:ctid=%ld => "f, __FUNCTION__, __LINE__,getpid(),gettid(),##__VA_ARGS__); printf("\n");

#define SL_LOGE(f,...) printf("%-15s:%04d:pid=%d:ctid=%ld => "f, __FUNCTION__, __LINE__,getpid(),gettid(),##__VA_ARGS__); printf("\n");

#define SL_LOGV(f,...) printf("%-15s:%04d:pid=%d:ctid=%ld => "f, __FUNCTION__, __LINE__,getpid(),gettid(),##__VA_ARGS__); printf("\n");

#endif



