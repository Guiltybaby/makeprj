#ifndef __ALOG_HEADER__
#define __ALOG_HEADER__

#include <stdio.h>
#include <aos.h>
#define A_LOGD(f,...) \
		printf("%-15s:%04d:pid=%d:ctid=%ld => "f"\n",__FUNCTION__, __LINE__,getpid(),(long int)gettid(),##__VA_ARGS__)

#define SL_LOGD(f,...) A_LOGD(f,##__VA_ARGS__)

#define SL_LOGE(f,...) A_LOGD(f,##__VA_ARGS__)

#define SL_LOGV(f,...) A_LOGD(f,##__VA_ARGS__)

#endif



