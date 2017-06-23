#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
//#include <sys/types.h>
#include <sys/syscall.h>
#include <string.h>

pthread_mutex_t gmutextest;
int time_count = 0;
void* threadfunc(void* param)
{
//	printf("PPID = %ld \n",pthread_self());
	long int a = (long)param;
	pthread_mutex_lock(&gmutextest);
	printf("pid = %d tid = %ld passing arg = %ld \n",getpid(),syscall(SYS_gettid),(long)param);
	sleep(1);
	time_count++;
	pthread_mutex_unlock(&gmutextest);
	pthread_exit((void*)a);
}

void test1()
{
	pthread_t ipd[5];
	void *status;
	int rc = 0;
	long t = 0;
	size_t stacksize = 0;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
	pthread_attr_getstacksize(&attr,&stacksize);
	printf("stacksize = %lu \n",stacksize);

	pthread_mutex_init(&gmutextest,NULL);

	for(t = 0 ; t < 5; t++)
	{
//	printf("creat thread = %ld\n",t);
		rc = pthread_create(&ipd[t],&attr,threadfunc,(void *)t);
		if(rc)
		{
			printf("thread create error rc = %d t = %ld",rc,t);
			exit(1);
		}
		else
		{
//			printf("thread create success \n");
		}
	}
	pthread_attr_destroy(&attr);
	for(t = 0 ; t < 5; t++)
	{
		if(ipd[t])
		{
			pthread_join(ipd[t],&status);
			printf("thread exit code = %ld \n",(long)status);
		}
	}
	pthread_mutex_init(&gmutextest,NULL);
	printf ("total time_count = %d \n",time_count);
	return;
}


pthread_mutex_t mutextest2;
pthread_cond_t condtest2;
int gcount = 0;
#define MAX_COUNT 0
void* thread_inc(void* argv)
{
	while(1)
	{
		pthread_mutex_lock(&mutextest2);

		gcount++;
		if(gcount > MAX_COUNT)
		{
			pthread_cond_signal(&condtest2);
		}
		printf("grap data \n");
		pthread_mutex_unlock(&mutextest2);
		sleep(1);
	}
	printf("inc thread");
	pthread_exit((void*)1);
}

void* thread_dec(void* argv)
{
	while(1)
	{
		pthread_mutex_lock(&mutextest2);
		pthread_cond_wait(&condtest2,&mutextest2);
		gcount = 0;
		printf("deliver data \n");
		pthread_mutex_unlock(&mutextest2);
	}
	printf("inc thread");
	pthread_exit((void*)2);
}
void test2()
{
	pthread_t ipd[2];
	int ret = 0;
	void* status;
	pthread_mutex_init(&mutextest2,NULL);
	pthread_cond_init(&condtest2,NULL);

	ret = pthread_create(&ipd[0],NULL,thread_inc,NULL);
	if(ret)
	{
		printf("creat thread2 error");
		exit(1);
	}
	ret = pthread_create(&ipd[1],NULL,thread_dec,NULL);
	if(ret)
	{
		printf("creat thread2 error");
		exit(1);
	}
	pthread_join(ipd[0],&status);
	printf("status = %ld",(long)status);
}

void test3()
{
//	0x1 0x2 0x1 0x2 0x1 0x2 0x1 0x20 0x30 0x40 0x50
//	0x1 0x2 0x1 0x2 0x3 0x4 0x5 0x20 0x30 0x40 0x50
	unsigned char a[10] = {0x1,0x2,0x3,0x4,0x5,0x10,0x20,0x30,0x40,0x50};
	unsigned char b[10] = {0x1,0x2,0x3,0x4,0x5,0x10,0x20,0x30,0x40,0x50};
	memcpy(&a[2],&a[0],5);
	printf("mem layout = 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x\n",a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7],a[8],a[9]);
	memmove(&b[2],&b[0],5);
	printf("mem layout = 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x\n",b[0],b[1],b[2],b[3],b[4],b[5],b[6],b[7],b[8],b[9]);


}

enum testenum_t{
	ONE = 1,
	TWO,
	THREE,
};

int main(int argc, char* argv[])
{
//	test1();
//	test2();
//	test3();
	printf("%lu \n",sizeof(enum testenum_t));
}
	


