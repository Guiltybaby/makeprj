
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <string.h>

#include "alog.h"
#include "base.h"

int client_creat()
{
	int sfd;
	ssize_t numRead;
	char buf[BUF_SIZE] = { 0 };
	const char * str = "hello world !";
	strncpy(buf, str, (((strlen(str)+1) < BUF_SIZE) ? (strlen(str)+1) : BUF_SIZE));
	buf[BUF_SIZE - 1] = '\0';
	sfd = socket_init(CLIENT);
	if(write(sfd, buf, BUF_SIZE) != BUF_SIZE)
	{
		A_LOGD("write error");
		assert(0);
	}
	close(sfd);
	exit(0);
}


int main(int argc,char *argv[])
{
	int sfd;
	ssize_t numRead;
	char buf[BUF_SIZE] = { 0 };
	signal(SIGPIPE,SIG_IGN);

	sfd = socket_init(CLIENT);
	int pid = getpid();
	memcpy(buf,&pid,sizeof(int));
	
	if(write(sfd, buf, BUF_SIZE) != BUF_SIZE)
	{
		A_LOGD("write error");
		assert(0);
	}
	A_LOGD("server pid = %d sfd = %d",pid,sfd);
	handler(pid,sfd,1);
#if 1
	sfd = socket_init(CLIENT);
	memcpy(buf,&pid,sizeof(int));
	if(write(sfd, buf, BUF_SIZE) != BUF_SIZE)
	{
		A_LOGD("write error");
		assert(0);
	}
	while(1)
	{
		sleep(5);
		const char * str = "client !";
		A_LOGD("sfd = %d client send",sfd);
		strncpy(buf, str, (((strlen(str)+1) < BUF_SIZE) ? (strlen(str)+1) : BUF_SIZE));
		buf[BUF_SIZE - 1] = '\0';
		int len = write(sfd, buf, BUF_SIZE);
		if(len != BUF_SIZE)
		{
			A_LOGD("write error len = %d BUF_SIZE = %d",len,BUF_SIZE);
			assert(0);
		}
	}
#endif
	while(1){
		sleep(1);
	}
}


