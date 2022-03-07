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
#define DEFAULT_PORT 8000
int readthemall(int);
int startsocket(int);
int search_one(void*, int, void*, int);




static char response[] =
	"HTTP/1.1 200 OK\r\n"
	"\r\n";
static char htmlpath[0x200];
static int htmlroot = 0;
static char codepath[0x200];
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

	j = 5;
	k = 5;
	while(1)
	{
		if(buf[j] <= 0x20)break;

		else if('%' != buf[j])
		{
			if(j != k)buf[k] = buf[j];
			j += 1;
		}
		else
		{
			if((buf[j+1] >= 'A')&&(buf[j+1] <= 'F'))
			{
				ret = (buf[j+1] - 'A' + 10)<<4;
			}
			else if((buf[j+1] >= '0')&&(buf[j+1] <= '9'))
			{
				ret = (buf[j+1] - '0')<<4;
			}
			else printf("error1:%x", buf[j+1]);

			if((buf[j+2] >= 'A')&&(buf[j+2] <= 'F'))
			{
				ret |= (buf[j+2] - 'A' + 10)&0xf;
			}
			else if((buf[j+2] >= '0')&&(buf[j+2] <= '9'))
			{
				ret |= (buf[j+2] - '0')&0xf;
			}
			else printf("error2:%x", buf[j+2]);

			buf[k] = ret;
			j += 3;
		}

		k++;
	}
	for(j=5;j<k;j++)printf("%02x",buf[j]);
	printf("\n");

	j = snprintf(wbuf, 0x80, "%s", response);
	if(buf[5] == '?')
	{
		ret = servesocket_search(wbuf+j, wlen, buf+6, k-6);
	}
	else if(buf[5] == '!')
	{
		ret = servesocket_code(wbuf+j, wlen, buf+6, k-6);
	}
	else
	{
		ret = servesocket_html(wbuf+j, wlen, buf+5, k-5);
	}

	if(ret <= 0)return 0;
	else return j+ret;
}




void serve(int argc, char** argv)
{
	int port;
	int j;
	char* p;

	port = DEFAULT_PORT;
	coderoot = snprintf(codepath, 0x200, "");
	htmlroot = snprintf(htmlpath, 0x200, "datafile/");
	for(j=1;j<argc;j++)
	{
		p = argv[j];
		if((p[0] >= '0')&&(p[0] <= '9'))
		{
			sscanf(argv[j], "%d", &port);
			continue;
		}
		if(0 == strncmp(p, "code=", 5))
		{
			coderoot = snprintf(codepath, 0x200, "%s", p+5);
			if(0 == coderoot)continue;
			if('/' == codepath[coderoot])continue;
			codepath[coderoot] = '/';
			coderoot++;
			continue;
		}
		if(0 == strncmp(p, "html=", 5))
		{
			htmlroot = snprintf(htmlpath, 0x200, "%s", p+5);
			if(0 == htmlroot)continue;
			if('/' == htmlpath[htmlroot])continue;
			htmlpath[htmlroot] = '/';
			htmlroot++;
			continue;
		}
	}
	printf(
		"port=%d, code=%s, html=%s\n",
		port, codepath, htmlpath
	);

	readthemall(1);
	startsocket(port);
}
