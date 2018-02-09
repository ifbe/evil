#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#ifndef O_BINARY
        #define O_BINARY 0x0
#endif
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long




struct pinindex
{
	u32 self;
	u32 what;
	u32 off;
	u32 len;

	u64 first;
	u64 last;
};
#define maxlen 0x100000
static u8 pinindexbuf[maxlen];
static int pinindexfd;
static int pinindexlen = 0;




void* pin_write()
{
	struct pinindex* addr;

	addr = (void*)pinindexbuf + pinindexlen;
	addr->self = pinindexlen;

	pinindexlen += 0x20;
	return addr;
}
void* pin_read(int offset)
{
	return (void*)pinindexbuf + offset;
}
void pinindex_start(int flag)
{
	int j;
	char* buf;
	char* name = ".42/pin/00";

	if(flag == 0)
	{
		pinindexfd = open(
			name,
			O_CREAT|O_RDWR|O_TRUNC|O_BINARY,
			S_IRWXU|S_IRWXG|S_IRWXO
		);
		pinindexlen = 0x20;

		buf = (void*)pinindexbuf;
		for(j=0;j<maxlen;j++)buf[j] = 0;
	}
	else
	{
		//open
		pinindexfd = open(
			name,
			O_CREAT|O_RDWR|O_BINARY, 
			S_IRWXU|S_IRWXG|S_IRWXO
		);

		//read
		pinindexlen = read(pinindexfd, pinindexbuf, maxlen);
		printf("pinindex:	%x\n", pinindexlen);

		//clean
		buf = (void*)pinindexbuf;
		for(j=pinindexlen;j<maxlen;j++)buf[j] = 0;
	}
}
void pinindex_stop()
{
	lseek(pinindexfd, 0, SEEK_SET);
	write(pinindexfd, pinindexbuf, pinindexlen);
	close(pinindexfd);
}
void pinindex_create()
{
}
void pinindex_delete()
{
}
