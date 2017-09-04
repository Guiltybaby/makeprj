#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "atask.h"
#include "asemaphore.h"
#include "alog.h"
#include "aos.h"

static Semaphore_t	sema_shared1 = NULL;
static Semaphore_t	sema_shared2 = NULL;

static void ipc_test_sharedmemory()
{
	A_LOGD("client enter");
	sleep(2);	
	int status, ret;
	struct stat buff;
	pid_t pid;
	if(sema_shared1 == NULL)
	{
		sema_shared1 = ASEMAPHORE_GetHandle_IPC((char *)"test.semaphore.shared1");
	}
	if(sema_shared2 == NULL)	
	{
		sema_shared2 = ASEMAPHORE_GetHandle_IPC((char *)"test.semaphore.shared2");
	}
	assert(sema_shared1);
	assert(sema_shared2);
	while(1)
	{
		A_LOGD("server sema_shared1 release");
		ASEMAPHORE_Release(sema_shared1);		
		sleep(1);
		A_LOGD("server sema_shared2 obtain");
		ASEMAPHORE_Obtain(sema_shared2, TICKS_FOREVER);		
	}
	A_LOGD("client exit");
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


