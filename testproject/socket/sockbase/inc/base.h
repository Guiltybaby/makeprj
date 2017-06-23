#ifndef __SOCKET_BASE_HEADER__
#define __SOCKET_BASE_HEADER__

#define BACKLOG 5
#define BUF_SIZE 255

typedef enum{
	CLIENT,
	SERVER
} UID;

int socket_init(UID uid);

void handler(int cpid,int cfd, int in);

#endif

