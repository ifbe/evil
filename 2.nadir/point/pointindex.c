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




struct pointindex
{
	u32 self;
	float x;
	float y;
	float z;

	u64 first;
	u64 last;
};
#define maxlen 0x100000
static u8 pointindexbuf[maxlen];
static int pointindexfd;
static int pointindexlen;




void* point_write(int num, double x, double y, double z)
{
	struct pointindex* addr;
	addr = (void*)pointindexbuf + num*0x20;

	addr->self = num*0x20;
	addr->x = x;
	addr->y = y;
	addr->z = z;

	//printf("%d	%lf, %lf, %lf\n", num, x, y, z);
	return addr;
}
void* point_read(int offset)
{
	return (void*)pointindexbuf + offset;
}
void pointindex_start(int flag)
{
	int j;
	char* buf;
	char* name = ".42/point/00";

	if(flag == 0)
	{
		pointindexfd = open(
			name,
			O_CREAT|O_RDWR|O_TRUNC|O_BINARY,
			S_IRWXU|S_IRWXG|S_IRWXO
		);
		pointindexlen = 0x1000;

		buf = (void*)pointindexbuf;
		for(j=0;j<maxlen;j++)buf[j] = 0;
	}
	else
	{
		//open
		pointindexfd = open(
			name,
			O_CREAT|O_RDWR|O_BINARY, 
			S_IRWXU|S_IRWXG|S_IRWXO
		);

		//read
		pointindexlen = read(pointindexfd, pointindexbuf, maxlen);
		printf("pointindex:	%x\n", pointindexlen);

		//clean
		buf = (void*)pointindexbuf;
		for(j=pointindexlen;j<maxlen;j++)buf[j] = 0;
	}
}
void pointindex_stop()
{
	lseek(pointindexfd, 0, SEEK_SET);
	write(pointindexfd, pointindexbuf, pointindexlen);
	close(pointindexfd);
}
void pointindex_create()
{
}
void pointindex_delete()
{
}