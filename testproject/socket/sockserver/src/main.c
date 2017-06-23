
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include "alog.h"
#include "base.h"


int server_creat()
{
	ssize_t numRead;
	int sfd,cfdin,cfdout;
	int cpid[10]={ 0 };
	char buf[BUF_SIZE];
	sfd = socket_init(SERVER);
	for(;;)
	{
		A_LOGD("accepted enter");
		cfdin = accept(sfd, NULL, NULL);
		A_LOGD("accepted1");
		if(cfdin == -1)
		{
			A_LOGD("accept error");
			assert(0);
		}
		numRead = read(cfdin, buf, BUF_SIZE);
		if(numRead <= 0){
			if(close(cfdin) == -1)			
			{
				A_LOGD("close error");
				assert(0);
			}
			break;
		}
		int *p = (int *)buf;
		int cpid = *p;
		A_LOGD("client cpid = %d,cfdin=%d",cpid,cfdin);
		handler(cpid,cfdin,0);
#if 1
		cfdout = accept(sfd, NULL, NULL);
		A_LOGD("accepted2");
		if(cfdout == -1)
		{
			A_LOGD("accept error");
			assert(0);
		}
		numRead = read(cfdout, buf, BUF_SIZE);
		if(numRead <= 0){
			if(close(cfdout) == -1)			
			{
				A_LOGD("close error");
				assert(0);
			}
			break;
		}
		p = (int *)buf;
		cpid = *p;
		A_LOGD("client pid = %d",cpid);
		handler(cpid,cfdout,1);
#endif
	}
	close(sfd);	
}

int main(int argc,char *argv[])
{
	signal(SIGPIPE,SIG_IGN);
	server_creat();
}
