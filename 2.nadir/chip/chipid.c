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




struct chipindex
{
	u32 self;
	u32 what;
	u32 off;
	u32 len;

	u64 first;
	u64 last;
};
#define maxlen 0x100000
static u8 chipindexbuf[maxlen];
static int chipindexfd;
static int chipindexlen;




void* chip_write(u8* buf, int len)
{
	struct chipindex* addr;
	//printf("chip@%d\n", linenum);

	addr = (void*)chipindexbuf + chipindexlen;
	addr->self = chipindexlen;

	chipindexlen += 0x20;
	if(chipindexlen >= maxlen)
	{
		printf("err@funcindex\n");
		return 0;
	}
	return addr;
}
void* chipindex_read(int offset)
{
	return (void*)chipindexbuf + offset;
}
void chipindex_start(int flag)
{
	int j;
	char* buf;
	char* name = ".42/chip/00";

	if(flag == 0)
	{
		chipindexfd = open(
			name,
			O_CREAT|O_RDWR|O_TRUNC|O_BINARY,
			S_IRWXU|S_IRWXG|S_IRWXO
		);
		chipindexlen = 0x20;

		buf = (void*)chipindexbuf;
		for(j=0;j<maxlen;j++)buf[j] = 0;
	}
	else
	{
		//open
		chipindexfd = open(
			name,
			O_CREAT|O_RDWR|O_BINARY, 
			S_IRWXU|S_IRWXG|S_IRWXO
		);

		//read
		chipindexlen = read(chipindexfd, chipindexbuf, maxlen);
		printf("chipindex:	%x\n", chipindexlen);

		//clean
		buf = (void*)chipindexbuf;
		for(j=chipindexlen;j<maxlen;j++)buf[j] = 0;
	}
}
void chipindex_stop()
{
	lseek(chipindexfd, 0, SEEK_SET);
	write(chipindexfd, chipindexbuf, chipindexlen);
	close(chipindexfd);
}
void chipindex_create()
{
}
void chipindex_delete()
{
}
