#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "base.h"
#include "alog.h"

const char* SOCKET_NAME = "/home/jeff_liu/prac/makefiletest/new_make/mysock";

int socket_init(UID uid)
{
	int sfd;
	struct sockaddr_un addr;
	sfd = socket(AF_UNIX, SOCK_STREAM, 0);	
	if(sfd == -1)
	{
		A_LOGD("socket creat error");
		assert(0);
	}
	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, SOCKET_NAME, sizeof(addr.sun_path) - 1);
	if(uid == CLIENT)
	{
		int timeout = 10;
		while(timeout-- > 0)
		{
			if(connect(sfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) == -1)			
			{
				A_LOGD("connect error no server exist sleep 1 s");
				sleep(1);
	//			assert(0);
			}
			else
			{
				break;
			}
		}
		if(timeout <= 0){
			close(sfd);
			assert(0);
		}
	}
	else if(uid == SERVER)
	{
		if(remove(SOCKET_NAME) == -1 && errno != ENOENT)
		{
			A_LOGD("remove error error num = %s",strerror(errno));
			assert(0);
		}
		if(bind(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1)
		{
			A_LOGD("bind error");
			assert(0);
		}
		if(listen(sfd,BACKLOG) == -1)
		{
			A_LOGD("lesten error");
			assert(0);
		}	
		A_LOGD("lesten out");
	}
	else
	{
		A_LOGD("param error");
		assert(0);
	}
	return sfd;	
}

void* worker_thread(long param){
	ssize_t numRead;
	char buf[BUF_SIZE];
	int* data = (int*)param;
	int cpid = data[0];
	int cfd = data[1];
	int in = data[2];
	free(param);
	A_LOGD("cpid =%d cfd = %d in = %d %x",cpid,cfd,in,param);
	if(in)
	{
		A_LOGD("server read");
		while(numRead = read(cfd, buf, BUF_SIZE) >= 0)
		{
			if(numRead > 0)
			{
				A_LOGD("cfd = %d cpid = %d buf = %s numRead = %d",cfd,cpid,buf,numRead);
				memset(buf,0,sizeof(buf));
			}
		}
		A_LOGD("numRead = %d",numRead);
	}
	else
	{
		while(1)
		{
			const char * str = "server !";
			A_LOGD("cfd = %d client send",cfd);
			strncpy(buf, str, (((strlen(str)+1) < BUF_SIZE) ? (strlen(str)+1) : BUF_SIZE));
			buf[BUF_SIZE - 1] = '\0';
			sleep(1);
			int len = write(cfd, buf, BUF_SIZE);
			if(len != BUF_SIZE)
			{
				A_LOGD("write error len = %d BUF_SIZE = %d",len,BUF_SIZE);
				if(len == -1)
				{
					A_LOGD("client dead");
					return;
				}
				assert(0);
			}
		}
	}
	A_LOGD("%d client disconnected",cpid);
	if(close(cfd) == -1)			
	{
		A_LOGD("close error");
		assert(0);
	}
}

void handler(int cpid,int cfd, int in){
	int* param = malloc(10);
	size_t stacksize = 0;
	pthread_t tid;
	pthread_attr_t attr;
	param[0] = cpid;
	param[1] = cfd;
	if(in)
	{
		param[2] = 1;
	}
	else
	{
		param[2] = 0;
	}
	A_LOGD("pid = %d sfd = %d,in = %d %x",param[0],param[1],param[2],param);
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
	pthread_attr_getstacksize(&attr,&stacksize);
	int rc = pthread_create(&tid,&attr,worker_thread,param);
	if(rc)
	{
		A_LOGD("thread create error rc = %d",rc);
		exit(1);
	}
	else
	{
		if(in)
			A_LOGD("creat server success");
		else
			A_LOGD("creat client success");
	}
}


