#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<fcntl.h>
#include<signal.h>
#include<unistd.h>
#include<signal.h>
#include<arpa/inet.h>
#include<linux/if_ether.h>
#include<net/if.h>
#include<netinet/in.h>
#include<netinet/ether.h>
#include<sys/epoll.h>
#include<sys/ioctl.h>
#include<sys/socket.h>
#include<sys/types.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#define MAXSIZE 4096
int servesocket(void*, int, void*, int);




static int epollfd;
static int tcpfd;
static u8 rbuf[0x100000];
static u8 wbuf[0x100000];




void epoll_add(u64 fd)
{
	int flag;
	struct epoll_event ev;
	flag = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flag | O_NONBLOCK);

	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = fd;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}
void epoll_del(u64 fd)
{
	struct epoll_event ev;

	ev.events = EPOLLIN;
	ev.data.fd = fd;
	epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
	close(fd);
}
void epoll_mod(u64 fd)
{
	struct epoll_event ev;

	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = fd;
	epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
}
int myaccept(int fd)
{
	int cc;
	while(1)
	{
		struct sockaddr_in haha;
		socklen_t len = sizeof(struct sockaddr_in);

		cc = accept(fd, (struct sockaddr*)&haha, &len);
		if(cc == -1)break;
		if(cc >= MAXSIZE)
		{
			printf("fd>MAXSIZE\n");
			close(cc);
			continue;
		}

		epoll_add(cc);
		//printf("++++ %d\n",cc);
	}//while
	return 0;
}
int myread(int fd)
{
	int ret = read(fd, rbuf, 0x100000);
	if(ret <= 0)return 0;

	ret = servesocket(wbuf, 0x100000, rbuf, ret);
	if(ret <= 0)return 0;

	ret = write(fd, wbuf, ret);
	return ret;
}
int listensocket()
{
	int j, fd, ret;
	struct epoll_event ev[16];

	while(1)
	{
		ret = epoll_wait(epollfd, ev, 16, -1);
		if(ret <= 0)continue;

		//printf("epoll:%d\n", ret);
		for(j=0; j<ret; j++)
		{
			fd = ev[j].data.fd;
			if(ev[j].events & EPOLLRDHUP)
			{
				printf("rdhup!\n");
			}//EPOLLRDHUP
			else if(ev[j].events & EPOLLIN)
			{
				if(fd == tcpfd)
				{
					myaccept(fd);
					epoll_mod(fd);
				}
				else
				{
					myread(fd);
					close(fd);
				}
			}//EPOLLIN
		}//for

		//wait for completion
		//usleep(100000);
	}//while

	printf("listensocket end\n");
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

	//epoll
	epollfd = epoll_create(MAXSIZE);
	if(epollfd <= 0)printf("%d,%d@epoll_create\n", epollfd, errno);
	epoll_add(tcpfd);

	//listen
	listen(tcpfd, 5);
	listensocket();
}
