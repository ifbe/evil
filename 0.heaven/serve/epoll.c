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
void readthemall(j);
void search_one(char* buf, int len);




static int epollfd;
static int tcpfd;
static u8 buffer[0x100000];
static u8 GET[0x1000];
static u8 response[] =
	"HTTP/1.1 200 OK\r\n"
	"Content-type: text/html\r\n"
	"\r\n";




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




int servesocket(char* buf, int len)
{
	int j,ret;
	char* p;
	//printf("%.*s\n\n", len, buf);

	if(strncmp(buf, "GET /", 5) != 0)return 0;

	p = buf+5;
	ret = 0;
	for(j=0;j<0x1000;j++)
	{
		if(p[j] <= 0x20)
		{
			p[j] = 0;
			ret = j;
			break;
		}
	}
	//printf("%.*s\n", ret, p);

	if(ret == 0)ret = snprintf(GET, 0x80, "index.html");
	else
	{
		for(j=0;j<ret;j++)GET[j] = p[j];
		GET[ret] = 0;

		search_one(GET, ret);
	}

	ret = snprintf(buf, 0x80, "%s%s", response, GET);
	return ret;
}
int listensocket()
{
	int j, tt, ret;
	int fd, cc;
	struct epoll_event epollevent[16];

	while(1)
	{
		tt = epoll_wait(epollfd, epollevent, 16, -1);	//start fetch
		if(tt <= 0)continue;

		//printf("epoll:%d\n", tt);
		for(j=0; j<tt; j++)
		{
			fd = epollevent[j].data.fd;
			if(epollevent[j].events & EPOLLRDHUP)printf("rdhup!!!!!!!\n");

			else if(epollevent[j].events & EPOLLIN)
			{
				//accept
				if(fd == tcpfd)
				{
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

					//reset tcpfd
					epoll_mod(fd);
				}//accept

				//read
				else
				{
ret = read(fd, buffer, 0x100000);
if(ret > 0)
{
	ret = servesocket(buffer, ret);
	if(ret > 0)
	{
		ret = write(fd, buffer, ret);
	}
}
close(fd);
				}
			}//EPOLLIN
		}//for

		//wait for completion
		//usleep(100000);
	}//while
}
int startsocket(int port)
{
	int ret;
	struct sockaddr_in self;

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

	//work
	listen(tcpfd, 5);

	//done
	epoll_add(tcpfd);
	return tcpfd;
}
void serve(int argc, char** argv)
{
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &sa, 0);

	readthemall(1);

	epollfd = epoll_create(MAXSIZE);
	if(epollfd <= 0)printf("%d,%d@epoll_create\n", epollfd, errno);

	startsocket(80);
	listensocket();
}
