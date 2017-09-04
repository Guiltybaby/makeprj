#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "asemaphore.h"
#include "alog.h"
#include "aos.h"

static Semaphore_t	sema_shared1 = NULL;
static Semaphore_t	sema_shared2 = NULL;

static void ipc_test_sharedmemory()
{
	int status = 0, ret = 0;
	struct stat buff;
	pid_t pid;
	aosstatus_t aosret = 0;
	const char* filepath1 = "/home/jeff_liu/prac/makefiletest/testproject/out/install/tmp/zshmem_key_1108987629";
	const char* filepath2 = "/home/jeff_liu/prac/makefiletest/testproject/out/install/tmp/stub.txt";
	A_LOGD("enter");
	if(stat(filepath1,&buff) == 0)
	{
		remove(filepath1);
	}
	if(stat(filepath2,&buff) == 0)
	{
		remove(filepath2);
	}
	if(sema_shared1 == NULL)
	{
		sema_shared1 = ASEMAPHORE_Create_IPC(
			(char *)"test.semaphore.shared1",
			1,
			0,
			OSSUSPEND_FIFO,
			0
			);
	}
	if(sema_shared2 == NULL)
	{
		sema_shared2 = ASEMAPHORE_Create_IPC(
			(char *)"test.semaphore.shared2",
			1,
			0,
			OSSUSPEND_FIFO,
			0
			);
	}
	assert(sema_shared1 == 0);
	assert(sema_shared2 == 0);
	while(1)
	{
		A_LOGD("server sema_shared2 release");
		aosret = ASEMAPHORE_Release(sema_shared2);		
		if(aosret){
			SL_LOGE("error ");
		}
		A_LOGD("server sema_shared1 obtain");
		ASEMAPHORE_Obtain(sema_shared1, TICKS_FOREVER);		
	}
	exit(1);
}

static void ipc_test()
{
	ipc_test_sharedmemory();
}

int main(int argc, char* argv[])
{
	ipc_test();
	return 22;
}


