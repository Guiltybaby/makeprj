#ifndef __TEST_CPP_HEADER__
#define __TEST_CPP_HEADER__

#include <sys/syscall.h>

#define gettid() syscall(SYS_gettid)
#define tgkill(pid,tid,signal) syscall(SYS_tgkill,pid,tid,signal)

#endif
