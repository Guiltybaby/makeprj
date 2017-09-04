#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>

#include "atask.h"
#include "asemaphore.h"
#include "alog.h"
#include "aos.h"

static Task_t		test_thread_one_task = NULL;
static Task_t		test_thread_two_task = NULL;
static Task_t		test_thread_three_task = NULL;
static pthread_t	test_posix_one_thread;
static Semaphore_t	sema_one = NULL;
static Semaphore_t	sema_two = NULL;
static Semaphore_t	sema_shared = NULL;

static void main_thread_loop();
static void test_thread_one_entry();
static void test_thread_two_entry();
static void test_thread_three_entry();
static int str_test();
static int file_test();
static int thread_test();
static int env_test();
static int proc_test();

extern char **environ;

void usage()
{
	printf("use the follow option \n");
	printf("-str \n");
	printf("-file \n");
	printf("-thread \n");
	printf("-env \n");
	printf("-proc \n");
	printf("-notify \n");
	printf("-ipc \n");
	printf("test -proc \n");
}


#include "test.h"


#define TEST_STR(symbol, name, arg) [ T_##symbol ] = { #symbol, name, arg, }, 
static struct {
	char* 	name;
	char* 	func;
	int   	arg;
} test[TEST_COUNT] = {
    [TEST_UNKNOWN] = { "unknow", "zxcv", 1 },
#include "test.h"
};
struct sized{
char a;
int  c;
long d;
long e;
};

static struct{
char d;
int a;
int b;
struct sized c;
}sizetest;

static int str_test()
{
	A_LOGD("enter ");
	char* tmp = "111"
		"123"
		"1233 ";
	int i;
	for(i = 0; i < TEST_COUNT; i++)
	{
		A_LOGD("name = %s ",test[i].name);
	}
	A_LOGD("tmp = %s ",tmp);
	A_LOGD("test size = %lu %lu %lu",sizeof(sizetest),sizeof(&sizetest),sizeof(struct sized));
	return 1;
}

#define __COREDUMP__

static void exit_status_handler(int status)
{
	
	if(WIFEXITED(status))
	{
		A_LOGD("child proc exit number = %d ",WEXITSTATUS(status));
	}
	else if(WIFSIGNALED(status))
	{
		A_LOGD("child proc exit signal = %d %s",WTERMSIG(status),
		#ifdef __COREDUMP__
			WCOREDUMP(status)?" core dump file":"");
		#else
			"");
		#endif
	}
	else if (WIFSTOPPED(status))
	{
		A_LOGD("child proc stop signal = %d",WSTOPSIG(status));
	}
		
//signal continue
//	WIFCONTINUED(status);
}

static int proc_test_thread()
{
	pid_t pid;
	if(sema_one == NULL)
	{
		sema_one = ASEMAPHORE_Create(
			(char *)"test.semaphore.one",
			1,
			0,
			OSSUSPEND_FIFO,
			0
			);
	}
	printf("\n\n\n\n");
	pthread_create(&test_posix_one_thread,NULL,(void*)test_thread_one_entry,NULL);	
	pid = fork();
	if(pid < 0)
	{
		A_LOGD("error");
	}
	else if(pid == 0)
	{
		A_LOGD("child");
		while(1);
	}
	else
	{
		A_LOGD("parent");
		while(1);
	}
	return 0;
}

static int proc_test()
{
	A_LOGD("enter");
	int status = 0;
	int ret = 0;
	pid_t pid = 0;
	pid = fork();
	if(pid < 0)
	{	
		A_LOGD("fork error");
	}
	else if(pid == 0)
	{
		A_LOGD("enter child proc pid = %d",getpid());
		char* arg[1];
		arg[0] = "-addr";
		if(execl("/home/jeff_liu/prac/makefiletest/testproject/out/install/testcpp",
			"/home/jeff_liu/prac/makefiletest/testproject/out/install/testcpp","-addr",(char*)0))
		{
			
			A_LOGD("error = %s",strerror(errno));
		}
		exit(7);
	}
	else
	{
		ret = wait(&status);
		if(ret != pid)
		{
			A_LOGD("wait error !!! ");
		}
		else
		{
			exit_status_handler(status);
		}
		A_LOGD("enter parent proc pid = %d ",getpid());
	}
	//proc_test_thread();
	return 1;
}

static int env_test()
{
	char **pptmp = NULL;
	char *ptmp = NULL;

	for(pptmp = environ; *pptmp != NULL; *pptmp++)
	{
		printf("%s \n",*pptmp);
	}
	ptmp = getenv("LOGNAME");
	if(ptmp)
	{
		printf("%s \n",ptmp);
	}
	return 1;
}

static int file_test()
{
	A_LOGD("enter");
	int fd = -1;
	int ret = -1;
	mode_t filemode = 0;	
	char* filepath = "practice.txt";
	char buff[10] = "111111";
	char buff2[10] = "222222";
	ret = access(filepath,F_OK);
	if(ret)
	{
		filemode = umask(filemode);
		A_LOGD("filemode = %o",filemode);
		fd = open(filepath,O_RDWR | O_EXCL | O_CREAT,00666);
		filemode = umask(filemode);
		A_LOGD("filemode = %o",filemode);
		ret = access(filepath,F_OK);
		if(ret)
		{
			A_LOGD("error = %s fd = %d",strerror(errno),fd);
		}
	}
	if(fd < 0)
	{	
		fd = open(filepath,O_RDWR);
	}
	if(fd < 0)
	{
		A_LOGD("error = %s fd = %d",strerror(errno),fd);
		return -1;
	}
	else
	{
		if(write(fd,buff,strlen(buff)) != strlen(buff))
		{
			A_LOGD("error = %s",strerror(errno));
			return -1;
		}
		if(lseek(fd,1000,SEEK_SET) == -1)
		{
			A_LOGD("error = %s",strerror(errno));
			return -1;
		}
		if(write(fd,buff2,strlen(buff2)) != strlen(buff2))
		{
			A_LOGD("error = %s",strerror(errno));
			return -1;
		}
		A_LOGD("fd = %d",fd);
		while(1);	
		close(fd);
		return 0;
	}
}
static int thread_test()
{
	A_LOGD("enter");
	if(sema_one == NULL)
	{
		sema_one = ASEMAPHORE_Create(
			(char *)"test.semaphore.one",
			1,
			0,
			OSSUSPEND_FIFO,
			0
			);
	}

	if(sema_two == NULL)
	{
		sema_two = ASEMAPHORE_Create(
			(char *)"test.semaphore.two",
			1,
			0,
			OSSUSPEND_FIFO,
			0
			);
	}
	

	if(test_thread_one_task == NULL)
	{
		test_thread_one_task = ATASK_CreateExt(
					(TEntry_t)(&test_thread_one_entry),
					(TArgv_t)NULL,
					(char *)"test_thread_one_entry",
					(TPriority_t)BELOW_NORMAL,
					2048,
					ATASK_SCHED_FIFO
					);
	}

	if(test_thread_two_task == NULL)
	{
		test_thread_two_task = ATASK_CreateExt(
					(TEntry_t)(&test_thread_two_entry),
					(TArgv_t)NULL,
					(char *)"test_thread_two_entry",
					(TPriority_t)BELOW_NORMAL,
					2048,
					ATASK_SCHED_FIFO
					);
	}
	if(test_thread_three_task == NULL)
    	{
        	test_thread_three_task = ATASK_CreateExt(
        	            		(TEntry_t)(&test_thread_three_entry),
        	            		(TArgv_t)NULL,
        	            		(char *)"test_thread_two_entry",
        	            		(TPriority_t)NORMAL,
        	            		2048,
        	            		ATASK_SCHED_FIFO
        				);
    }
	assert(sema_one);
	assert(sema_two);
	assert(test_thread_one_task);
	assert(test_thread_two_task);
	assert(test_thread_three_task);

	main_thread_loop();
	return 1;
}

static void main_thread_loop()
{
	while(1)
	{
		sleep(1);
	}
}

static void test_thread_one_entry()
{
	while(1)
	{
		ASEMAPHORE_Release(sema_one);
		A_LOGD("1111111");
		sleep(1);
	}
}

static void test_thread_two_entry()
{
	while(1)
	{
		ASEMAPHORE_Obtain(sema_one, TICKS_FOREVER);
		A_LOGD("222222");
	}
}

static void test_thread_three_entry()
{
    while(1)
    {
	ASEMAPHORE_Obtain(sema_one, TICKS_FOREVER);
        A_LOGD("33333");
    }
}


#include <sys/inotify.h>
static void print_notify(struct inotify_event* i)
{
	A_LOGD("wd = %2d",i->wd)
	if(i->mask & IN_ACCESS) A_LOGD("mask = IN_ACCESS");
	if(i->mask & IN_ALL_EVENTS) A_LOGD("mask = IN_ALL_EVENTS");

	if(i->mask ) A_LOGD("mask = %08x",i->mask);

	if(i->len)
		A_LOGD("name = %s",i->name);
	
}
#define BUF_LEN (10 * sizeof(struct inotify_event) + 255 + 1)
static void notify_test()
{
	int fd;
	size_t readnum;
	char* ptmp;
	char* buff = (char *)malloc(BUF_LEN);
	struct inotify_event* event;
	fd = inotify_init();
	if(fd == -1)
	{
		A_LOGD("error exit");
		if(buff)
		{
			free(buff);
		}		
		exit(-1);
	}
	inotify_add_watch(fd,"/home/jeff_liu/prac/makefiletest/test",IN_ALL_EVENTS);
	for(;;)
	{
		readnum = read(fd,buff,BUF_LEN);
		if(readnum == 0 || readnum ==-1)
		{
			A_LOGD("error exit");
			if(buff)
			{
				free(buff);
			}		
			exit(-1);
		}
		A_LOGD("readnum = %ld",(long)readnum);
		for(ptmp = buff; ptmp < buff + BUF_LEN; )
		{
			event = (struct inotify_event*)ptmp;
			print_notify(event);
			ptmp += sizeof(struct inotify_event) + event->len;
		}
	}

}
#define MAX_LINE 255
static void ipc_test_pipe()
{
	int n,ret,status = 0 ;
	int fd[2];
	pid_t pid;
	char line[MAX_LINE] = {0};
	const char* tmp = "hello world";
	if(pipe(fd))
	{
		A_LOGD("ERROR");
	}
	if((pid = fork()) < 0)
	{	
		A_LOGD("ERROR");		
	}
	else if(pid > 0)
	{
		close(fd[1]);
		n = read(fd[0], line, MAX_LINE);
		line[MAX_LINE - 1] = '\0';
		A_LOGD("parent read = %s",line);		
		ret = wait(&status);
		if(ret != pid)
		{
			A_LOGD("wait error !!!");
		}
		else
		{
			exit_status_handler(status);
		}
		A_LOGD("parent proc exit pid = %d",getpid());
	}
	else
	{
		sleep(1);
		close(fd[0]);
		write(fd[1],tmp,strlen(tmp));
		A_LOGD("child write = %s",tmp);		
		A_LOGD(" proc exit pid = %d",getpid());
		//write(STDOUT_FILENO, line, MAX_LINE);
		exit(0);
	}
}

static void ipc_test_sharedmemory()
{
	A_LOGD("enter");
	int status, ret;
	struct stat buff;
	pid_t pid;
	const char* filepath1 = "/home/jeff_liu/prac/makefiletest/testproject/out/install/tmp/zshmem_key_1108987629";
	const char* filepath2 = "/home/jeff_liu/prac/makefiletest/testproject/out/install/tmp/stub.txt";
	if(stat(filepath1,&buff) == 0)
	{
		remove(filepath1);
	}
	if(stat(filepath2,&buff) == 0)
	{
		remove(filepath2);
	}
	if(sema_shared == NULL)
	{
		sema_shared = ASEMAPHORE_Create_IPC(
			(char *)"test.semaphore.shared",
			1,
			0,
			OSSUSPEND_FIFO,
			0
			);
	}
	pid = fork();
	if(pid < 0)
	{
		A_LOGD("fork error");
		exit(0);
	}
	else if(pid > 0)
	{
		A_LOGD("server enter sema_shared = %p",sema_shared)
		ASEMAPHORE_Obtain(sema_shared, TICKS_FOREVER);		
		A_LOGD("server obtained")
		ret = wait(&status);
		if(ret != pid)
		{
			A_LOGD("wait error !!!");
		}
		else
		{
			exit_status_handler(status);
		}
		A_LOGD("server proc exit pid = %d",getpid());
		
	}
	else
	{
		A_LOGD("child enter")
		sleep(5);	
		ASEMAPHORE_Release(sema_shared);		
		A_LOGD("child exit")
		while(1)
		sleep(1);	
		exit(1);
	}
}

static void ipc_test()
{
	ipc_test_pipe();
	ipc_test_sharedmemory();
}

static void ioctl_test()
{
	unsigned int a = -1;
	printf("a = 0x%08x",a);
}

typedef struct TEEC_UUID {
    uint32_t timeLow;
    uint16_t timeMid;
    uint16_t timeHiAndVersion;
    uint8_t clockSeqAndNode[8];
} TEEC_UUID;

static inline unsigned char hexchartovalue(char a)
{
	unsigned char b;
	if(a >= '0' && a <= '9')
	{
		b = a - '0';
	}
	else if(a >= 'a' && a <= 'f')
	{
		b = a - 'a' + 10;
	}
	else if(a >= 'A' && a <= 'F')
	{
		b = a - 'A' + 10;
	}
	else
	{
		SL_LOGE("error a = %c \n",a);
		return -1;
	}
	return b;
}

static int ParseArg(char *str, TEEC_UUID *_puuid)
{
	int i,len;
	unsigned char a,b;
	char* strptr = str;
	char* ptr = NULL;
	len = strlen(str);
	unsigned char buff[sizeof(TEEC_UUID)];
	memset(buff,0,sizeof(TEEC_UUID));
	if(len != sizeof(TEEC_UUID) * 2)	
	{
SL_LOGE("invalid UUID please check the init.rc len = %d TEEC_UUID = %lu",len,sizeof(TEEC_UUID));
		return -1;
	}
	else
	{
		for(i = 0; i < sizeof(TEEC_UUID); i++)
		{
			a = hexchartovalue(*strptr++);
			b = hexchartovalue(*strptr++);
			if(a == -1 || b == -1)
			{
				SL_LOGE("invalid UUID please check the init.rc");
				return -1;
			}
			buff[i] = a<<4 | b;
		}
		//parse timeLow
		_puuid->timeLow = buff[3] | buff[2]<<8 | buff[1]<<16 | buff[0]<<24;
		//parse timeMid
		_puuid->timeMid = buff[5] | buff[4]<<8;
		//parse timeHiAndVersion
		_puuid->timeHiAndVersion = buff[7] | buff[6]<<8;
		//parse clockSeqAndNode
		ptr = (char*)&_puuid->clockSeqAndNode;
		for(i = 0; i < sizeof(_puuid->clockSeqAndNode); i++)
		{
			ptr[i] = buff[8 + i];
		}
	}
	printf("uuid = %4x%2x%2x",_puuid->timeLow,_puuid->timeMid,_puuid->timeHiAndVersion);
	for(i = 0;i < sizeof(_puuid->clockSeqAndNode); i ++)
		printf("%2x",_puuid->clockSeqAndNode[i]);
	printf("\n");
	return 0;
}

int GslParseUuid(int argc, char *args[], TEEC_UUID *_puuid)
{
	int i,ret = -1;
	for(i = 1; i < argc; i++)
	{
		if(strcmp(args[i],"-uuid") == 0)
		{
			i++;
			if(i < argc)
			{
				if(ParseArg(args[i],_puuid))
				{
					SL_LOGD("parse uuid fail continue");
					continue;
				}
				else
				{
					SL_LOGD("parse uuid success");
					ret = 0;
					break;
				}
			}
			else
			{
				SL_LOGE("no uuid avalid please check the init.rc");
			}
		}
	}
	
	if(ret)
		SL_LOGE("no uuid avalid please check the init.rc");
	return ret;
}
#define SL_LOG_BUFSIZE 1024

static char buf[SL_LOG_BUFSIZE+1];
int sl_log_printf(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, SL_LOG_BUFSIZE, fmt, ap);
//	printf(fmt,ap);
	va_end(ap);
	printf("%s",buf);
	return 0;
}

#define SL_LOGL(f,...)	\
	do {	\
	sl_log_printf("func = %-15s \n",__FUNCTION__); \
	sl_log_printf("line = %04d \n",__LINE__ ); \
	sl_log_printf("SLCODE D %-15s:%04d => "f, __FUNCTION__, __LINE__,##__VA_ARGS__);	\
	sl_log_printf("\n"); \
	} while(0)

int main(int argc, char* argv[])
{
	int i;
	int usageflag = 0;
	TEEC_UUID uuid;
	GslParseUuid(argc,argv,&uuid);
	SL_LOGL("123123%s%d","asdf",234);
	for(i = 0; i < argc; i++)
	{
	printf("argv[%d] = %s \n",i,argv[i]);
/*
		if(strcmp(argv[i],"-str") == 0)
		{
			usageflag = 1;
			str_test();
			break;
		}
		else if(strcmp(argv[i],"-thread") == 0)
		{
			usageflag = 1;
			thread_test();
			break;
		}
		else if(strcmp(argv[i],"-file") == 0)
		{
			usageflag = 1;
			file_test();
			break;
		}
		else if(strcmp(argv[i],"-env") == 0)
		{
			usageflag = 1;
			env_test();
			break;
		}
		else if(strcmp(argv[i],"-proc") == 0)
		{
			usageflag = 1;
			proc_test();
			break;
		}
		else if(strcmp(argv[i],"-notify") == 0)
		{
			usageflag = 1;
			notify_test();
			break;
		}
		else if(strcmp(argv[i],"-ipc") == 0)
		{
			usageflag = 1;
			ipc_test();
			break;
		}
		else if(strcmp(argv[i],"-ioctl") == 0)
		{
			usageflag = 1;
			ioctl_test();
			break;
		}
*/
	}	
	if(!usageflag)
	{
//		usage();
	}
	return 22;
}


