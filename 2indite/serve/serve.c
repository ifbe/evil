#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
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
static u8 htmlpath[0x200];
static int htmlroot = 0;
static u8 codepath[0x200];
static int coderoot = 0;




int servesocket_html(char* wbuf, int wlen, char* buf, int len)
{
	int fd,ret = 0;
	char path[0x1000];

	ret = snprintf(path, 0x1000, "%.*s", htmlroot, htmlpath);
	if(len == 0)snprintf(path+ret, 0x1000, "index.html");
	else snprintf(path+ret, 0x1000, "%.*s", len, buf);

	fd = open(path, O_RDONLY);
	if(fd <= 0)return 0;

	ret = read(fd, wbuf, wlen);
	if(ret <= 0)
	{
		close(fd);
		return 0;
	}

	close(fd);
	return ret;
}
int servesocket_code(char* wbuf, int wlen, char* buf, int len)
{
	int fd,ret = 0;
	char path[0x1000];
	if(len == 0)return 0;

	ret = snprintf(path, 0x1000, "%.*s", coderoot, codepath);
	snprintf(path+ret, 0x1000, "%.*s", len, buf);

	fd = open(path, O_RDONLY);
	if(fd <= 0)return 0;

	ret = read(fd, wbuf, wlen);
	if(ret <= 0)
	{
		close(fd);
		return 0;
	}

	close(fd);
	return ret;
}
int servesocket_search(char* wbuf, int wlen, char* buf, int len)
{
	return search_one(wbuf, wlen, buf, len);
}
int servesocket(char* wbuf, int wlen, char* buf, int len)
{
	int j,k,ret;
	printf("%.*s", len, buf);
	if(strncmp(buf, "GET /", 5) != 0)
	{
		printf("error@GET\n");
		return 0;
	}

	for(j=5;j<len;j++)
	{
		if(buf[j] <= 0x20)break;
	}

	k = snprintf(wbuf, 0x80, "%s", response);
	if(buf[5] == '?')
	{
		ret = servesocket_search(wbuf+k, wlen, buf+6, j-6);
	}
	else if(buf[5] == '!')
	{
		ret = servesocket_code(wbuf+k, wlen, buf+6, j-6);
	}
	else
	{
		ret = servesocket_html(wbuf+k, wlen, buf+5, j-5);
	}

	if(ret <= 0)return 0;
	else return k+ret;
}
void serve(int argc, char** argv)
{
	int port;
	if(argc == 1)port = 80;
	else sscanf(argv[1], "%d", &port);
	htmlroot = snprintf(htmlpath, 0x200, "datafile/");
	coderoot = 0;

	readthemall(1);
	startsocket(port);
}
