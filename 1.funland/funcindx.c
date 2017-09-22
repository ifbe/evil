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
struct funcindex
{
	u32 self;
	u32 what;
	u32 off;
	u32 len;

	u64 first;
	u64 last;
};
static u8 funcindxbuf[0x100000];
static int funcindxfd;
static int funcindxlen;




void* funcindx_read()
{
}
void* funcindx_write(int linenum)
{
	struct funcindex* addr;
	//printf("func@%d\n", linenum);

	addr = (void*)funcindxbuf + funcindxlen;
	addr->self = funcindxlen;
	addr->what = linenum;

	funcindxlen += 0x20;
	return addr;
}
void funcindx_start()
{
	funcindxlen = 0x20;
}
void funcindx_stop()
{
}
void funcindx_create()
{
	int j;
	char* buf;

	buf = (void*)funcindxbuf;
	for(j=0;j<0x100000;j++)buf[j] = 0;

	//func
	funcindxfd = open(
		".42/func.index",
		O_CREAT|O_RDWR|O_BINARY,	//O_CREAT|O_RDWR|O_TRUNC|O_BINARY,
		S_IRWXU|S_IRWXG|S_IRWXO
	);

	//
	funcindxlen = read(funcindxfd, funcindxbuf, 0x100000);
	printf("funcindx:	%x\n", funcindxlen);
}
void funcindx_delete()
{
	lseek(funcindxfd, 0, SEEK_SET);
	write(funcindxfd, funcindxbuf, funcindxlen);
	close(funcindxfd);
}
