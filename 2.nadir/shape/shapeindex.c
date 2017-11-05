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
int decstr2data(void*, void*);




struct shapeindex
{
	u32 self;
	u32 x;
	u32 y;
	u32 z;

	u64 first;
	u64 last;
};
#define maxlen 0x100000
static u8 shapeindexbuf[maxlen];
static int shapeindexfd;
static int shapeindexlen = 0x20;




void* shape_write()
{
	struct shapeindex* addr;
	addr = (void*)shapeindexbuf + shapeindexlen;

	addr->self = shapeindexlen;

	shapeindexlen += 0x20;
	return addr;
}
void* shape_read(int offset)
{
	return (void*)shapeindexbuf + offset;
}
void shapeindex_start(int flag)
{
	int j;
	char* buf;
	char* name = ".42/shape/00";

	if(flag == 0)
	{
		shapeindexfd = open(
			name,
			O_CREAT|O_RDWR|O_TRUNC|O_BINARY,
			S_IRWXU|S_IRWXG|S_IRWXO
		);
		shapeindexlen = 0x20;

		buf = (void*)shapeindexbuf;
		for(j=0;j<maxlen;j++)buf[j] = 0;
	}
	else
	{
		//open
		shapeindexfd = open(
			name,
			O_CREAT|O_RDWR|O_BINARY, 
			S_IRWXU|S_IRWXG|S_IRWXO
		);

		//read
		shapeindexlen = read(shapeindexfd, shapeindexbuf, maxlen);
		printf("shapeindex:	%x\n", shapeindexlen);

		//clean
		buf = (void*)shapeindexbuf;
		for(j=shapeindexlen;j<maxlen;j++)buf[j] = 0;
	}
}
void shapeindex_stop()
{
	lseek(shapeindexfd, 0, SEEK_SET);
	write(shapeindexfd, shapeindexbuf, shapeindexlen);
	close(shapeindexfd);
}
void shapeindex_create()
{
}
void shapeindex_delete()
{
}