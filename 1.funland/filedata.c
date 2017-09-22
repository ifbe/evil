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
static u8 filebuf[0x100000];
static int filefd;
static int filelen;




void filedata_start()
{
	filelen = 0;
}
void filedata_stop()
{
}
void filedata_create()
{
	int j;
	char* buf;

	buf = (void*)filebuf;
	for(j=0;j<0x100000;j++)buf[j] = 0;

	//func
	filefd = open(
		".42/file.data",
		O_CREAT|O_RDWR|O_BINARY,	//O_CREAT|O_RDWR|O_TRUNC|O_BINARY,
		S_IRWXU|S_IRWXG|S_IRWXO
	);

	//
	filelen = read(filefd, filebuf, 0x100000);
	printf("filedata:	%x\n", filelen);
}
void filedata_delete()
{
	lseek(filefd, 0, SEEK_SET);
	write(filefd, filebuf, filelen);
	close(filefd);
}
