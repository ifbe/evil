#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<dirent.h>
#include<errno.h>
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
struct fileindex
{
	u32 self;
	u32 what;
	u32 off;
	u32 len;

	u64 first;
	u64 last;
};
#define maxlen 0x100000
static u8 md5buf[maxlen];
static int md5fd;
static int md5len;




void* filemd5_write(char* buf, int len)
{
	struct fileindex* addr;

	addr = (void*)md5buf + md5len;
	addr->self = md5len;
	addr->what = len;

	md5len += 0x20;
	if(md5len >= maxlen)
	{
		printf("err@filemd5\n");
		return 0;
	}
	return addr;
}
void* filemd5_read(int offset)
{
	return (void*)md5buf + offset;
}
void filemd5_start(int flag)
{
	int j;
	char* buf;
	char* name = ".42/file/00";

	if(flag == 0)
	{
		md5fd = open(
			name,
			O_CREAT|O_RDWR|O_TRUNC|O_BINARY,
			S_IRWXU|S_IRWXG|S_IRWXO
		);
		md5len = 0x20;

		buf = (void*)md5buf;
		for(j=0;j<maxlen;j++)buf[j] = 0;
	}
	else
	{
		//open
		md5fd = open(
			name,
			O_CREAT|O_RDWR|O_BINARY,
			S_IRWXU|S_IRWXG|S_IRWXO
		);

		//read
		md5len = read(md5fd, md5buf, maxlen);
		printf("filemd5:	%x\n", md5len);

		//clean
		buf = (void*)md5buf;
		for(j=md5len;j<maxlen;j++)buf[j] = 0;
	}
}
void filemd5_stop()
{
	lseek(md5fd, 0, SEEK_SET);
	write(md5fd, md5buf, md5len);
	close(md5fd);
}
void filemd5_create()
{
}
void filemd5_delete()
{
}
