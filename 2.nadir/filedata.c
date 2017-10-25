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




//
static u8 filedatabuf[0x100000];
static int filedatafd;
static int filedatalen;




void filedata_start(int flag)
{
	int j;
	char* buf;
	char* name = ".42/filedata/00";

	if(flag == 0)
	{
		filedatafd = open(
			name,
			O_CREAT|O_RDWR|O_TRUNC|O_BINARY,
			S_IRWXU|S_IRWXG|S_IRWXO
		);
		filedatalen = 0;

		buf = (void*)filedatabuf;
		for(j=0;j<0x100000;j++)buf[j] = 0;
	}
	else
	{
		//open
		filedatafd = open(
			name,
			O_CREAT|O_RDWR|O_BINARY,
			S_IRWXU|S_IRWXG|S_IRWXO
		);

		//read
		filedatalen = read(filedatafd, filedatabuf, 0x100000);
		printf("filedata:	%x\n", filedatalen);

		//clean
		buf = (void*)filedatabuf;
		for(j=filedatalen;j<0x100000;j++)buf[j] = 0;
	}
}
void filedata_stop()
{
	lseek(filedatafd, 0, SEEK_SET);
	write(filedatafd, filedatabuf, filedatalen);
	close(filedatafd);
}
void filedata_create()
{
}
void filedata_delete()
{
}
