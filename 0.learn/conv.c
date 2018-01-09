#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#define u64 unsigned long long
#define u32 unsigned int
#define u16 unsigned short
#define u8 unsigned char
#ifndef O_BINARY
	#define O_BINARY 0x0
#endif
#define hex32(a,b,c,d) (a | (b<<8) | (c<<16) | (d<<24))
#define hex64(a,b,c,d,e,f,g,h) (hex32(a,b,c,d) | (((u64)hex32(e,f,g,h))<<32))




int att2ac(int argc, char** argv)
{
	int j,fd,len;
	char* buf = 0;

	fd = open(argv[1], O_RDONLY|O_BINARY);
	if(fd <= 0)return -1;

	buf = malloc(0x100000);
	if(buf == 0)return -2;

	len = read(fd, buf, 0x100000);
	if(len <= 0)return -3;

	close(fd);
}
int intel2ac(int argc, char** argv)
{
	int j,fd,len;
	char* buf = 0;

	fd = open(argv[1], O_RDONLY|O_BINARY);
	if(fd <= 0)return -1;

	buf = malloc(0x100000);
	if(buf == 0)return -2;

	len = read(fd, buf, 0x100000);
	if(len <= 0)return -3;

	close(fd);
}




int ac2att(int argc, char** argv)
{
	int j,fd,len;
	char* buf = 0;

	fd = open(argv[1], O_RDONLY|O_BINARY);
	if(fd <= 0)return -1;

	buf = malloc(0x100000);
	if(buf == 0)return -2;

	len = read(fd, buf, 0x100000);
	if(len <= 0)return -3;

	close(fd);
}




void ac2intel_line(char* buf, int len)
{
	int j;
	for(j=0;j<len;j++)
	{
		if(buf[j] == '=')
		{
			printf("mov ");
			buf[j] = ',';
		}
		else if(buf[j] == '?')
		{
			printf("cmp ");
			buf[j] = ',';
		}
		else if(buf[j] == '<')
		{
			printf("shl ");
			buf[j] = ',';
		}
		else if(buf[j] == '>')
		{
			printf("shr ");
			buf[j] = ',';
		}
		else if(buf[j] == '+')
		{
			printf("add ");
			buf[j] = ',';
		}
		else if(buf[j] == '-')
		{
			printf("sub ");
			buf[j] = ',';
		}
		else if(buf[j] == '*')
		{
			printf("imul ");
			buf[j] = ',';
		}
		else if(buf[j] == '/')
		{
			printf("idiv ");
			buf[j] = ',';
		}
	}
	printf("%.*s\n",len,buf);
}
int ac2intel(int argc, char** argv)
{
	int j,k,fd,len;
	char* buf = 0;

	fd = open(argv[1], O_RDONLY|O_BINARY);
	if(fd <= 0)return -1;

	buf = malloc(0x100000);
	if(buf == 0)return -2;

	len = read(fd, buf, 0x100000);
	if(len <= 0)return -3;

	close(fd);

	printf("[bits 64]\n");
	k = 0;
	for(j=0;j<=len;j++)
	{
		if((j==len) | (buf[j]=='\n'))
		{
			ac2intel_line(buf+k, j-k);
			k=j+1;
		}
	}
}




int conv(int argc, char** argv)
{
	if(argc <= 2)return 0;

	if(strcmp(argv[1], "att2ac") == 0)
	{
		att2ac(argc-1, argv+1);
	}
	else if(strcmp(argv[1], "intel2ac") == 0)
	{
		intel2ac(argc-1, argv+1);
	}
	else if(strcmp(argv[1], "ac2att") == 0)
	{
		ac2att(argc-1, argv+1);
	}
	else if(strcmp(argv[1], "ac2intel") == 0)
	{
		ac2intel(argc-1, argv+1);
	}
}