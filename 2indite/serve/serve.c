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
int readthemall(int);
int startsocket(int);
int search_one(void*, int, void*, int);




static u8 response[] =
	"HTTP/1.1 200 OK\r\n"
	"\r\n";




int servesocket_url(char* wbuf, int wlen, char* buf, int len)
{
	int j,fd,ret = 0;
	for(j=0;j<len;j++)
	{
		if(buf[j] <= 0x20)
		{
			buf[j] = 0;
			ret = j;
			break;
		}
	}

	if(ret == 0)
	{
		fd = open("datafile/index.html", O_RDONLY);
		if(fd <= 0)return 0;
	}
	else
	{
		fd = open(buf, O_RDONLY);
		if(fd <= 0)return 0;
	}

	j = snprintf(wbuf, 0x80, "%s", response);
	ret = read(fd, wbuf+j, wlen-j);
	if(ret <= 0)
	{
		close(fd);
		return 0;
	}
	j += ret;

	close(fd);
	return j;
}
int servesocket_search(char* wbuf, int wlen, char* buf, int len)
{
	int j,ret = 0;
	for(j=0;j<0x1000;j++)
	{
		if(buf[j] <= 0x20)
		{
			buf[j] = 0;
			ret = j;
			break;
		}
	}
	j = snprintf(wbuf, 0x80, "%s", response);
	j += search_one(wbuf+j, wlen-j, buf, ret);
	return j;
}
int servesocket(char* wbuf, int wlen, char* buf, int len)
{
	printf("%.*s", len, buf);
	if(strncmp(buf, "GET /", 5) != 0)
	{
		printf("error@GET\n");
		return 0;
	}

	if(buf[5] != '?')
	{
		return servesocket_url(wbuf, wlen, buf+5, len-5);
	}
	else
	{
		return servesocket_search(wbuf, wlen, buf+6, len-6);
	}
}
void serve(int argc, char** argv)
{
	int port;
	if(argc == 1)port = 80;
	else
	{
		sscanf(argv[1], "%d", &port);
	}

	readthemall(1);
	startsocket(port);
}
