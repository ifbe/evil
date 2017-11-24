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




struct vertex
{
	float x;
	float y;
	float z;
	float w;
};
#define maxlen 0x100000
static u8 pointdatabuf[maxlen];
static int pointdatafd;
static int pointdatalen;




void* pointdata_read(int n)
{
	return (void*)pointdatabuf + n*0x10;
}
int pointdata_write(float x, float y, float z, float w)
{
	int j;
	struct vertex* p;
	//printf("%f,%f,%f,%f\n",x,y,z,w);

	j = pointdatalen;
	pointdatalen += 0x10;
	p = (void*)pointdatabuf + j;
	p->x = x;
	p->y = y;
	p->z = z;
	p->w = w;

	return j;
}
void pointdata_start(int flag)
{
	int j;
	char* buf;
	char* name = ".42/point/point.bin";

	if(flag == 0)
	{
		pointdatafd = open(name,
			O_CREAT|O_RDWR|O_TRUNC|O_BINARY,
			S_IRWXU|S_IRWXG|S_IRWXO
		);

		buf = (void*)pointdatabuf;
		for(j=0;j<maxlen;j++)buf[j] = 0;
	}
	else
	{
		//open
		pointdatafd = open(name,
			O_CREAT|O_RDWR|O_BINARY, 
			S_IRWXU|S_IRWXG|S_IRWXO
		);

		//read
		pointdatalen = read(pointdatafd, pointdatabuf, maxlen);
		printf("pointdata:	%x\n", pointdatalen);

		//clean
		buf = (void*)pointdatabuf;
		for(j=pointdatalen;j<maxlen;j++)buf[j] = 0;
	}
}
void pointdata_stop()
{
	lseek(pointdatafd, 0, SEEK_SET);
	write(pointdatafd, pointdatabuf, pointdatalen);
	close(pointdatafd);
}
void pointdata_create()
{
}
void pointdata_delete()
{
}