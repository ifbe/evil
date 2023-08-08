#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#define MAXSIZE 4096
int servesocket(void*, int, void*, int);




static int epollfd;
static int tcpfd;




int listensocket()
{
	return 0;
}
int startsocket(int port)
{
	int ret;
	struct sockaddr_in self;
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &sa, 0);

	//create
	tcpfd = socket(AF_INET, SOCK_STREAM, 0);
	if (tcpfd == -1)
	{
		printf("error@socket\n");
		return 0;
	}
	if(tcpfd > MAXSIZE)
	{
		printf("tcpfd>4096\n");
		return 0;
	}

	//reuse
	ret = 1;
	ret = setsockopt(tcpfd, SOL_SOCKET, SO_REUSEADDR, &ret, 4);
	if(ret<0)
	{
		printf("error@setsockopet\n");
		return 0;
	}

	//self
	memset(&self, 0, sizeof(struct sockaddr_in));
	self.sin_family = AF_INET;
	self.sin_port = htons(port);
	self.sin_addr.s_addr = htons(INADDR_ANY);
	ret = bind(tcpfd, (void*)&self, sizeof(struct sockaddr_in));
	if(ret == -1)
	{
		printf("error@bind\n");
		close(tcpfd);
		return 0;
	}

	//listen
	listen(tcpfd, 5);

	//kqueue
	listensocket();

	return 0;
}