#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "evil.h"
#ifndef O_BINARY
        #define O_BINARY 0x0
#endif
#define maxlen 0x1000000




static u8 funcindexbuf[maxlen];
static int funcindexfd;
static int funcindexlen;




void* funcindex_write(int linenum)
{
	struct funcindex* addr;
	//printf("func@%d\n", linenum);

	addr = (void*)funcindexbuf + funcindexlen;
	addr->self = funcindexlen;
	addr->what = linenum;

	funcindexlen += 0x20;
	if(funcindexlen >= maxlen)
	{
		printf("err@funcindex\n");
		return 0;
	}
	return addr;
}
void* funcindex_read(int offset)
{
	return (void*)funcindexbuf + offset;
}
void funcindex_start(int flag)
{
	int j;
	char* buf;
	char* name = ".42/func/00";

	if(flag == 0)
	{
		funcindexfd = open(
			name,
			O_CREAT|O_RDWR|O_TRUNC|O_BINARY,
			S_IRWXU|S_IRWXG|S_IRWXO
		);
		funcindexlen = 0x20;

		buf = (void*)funcindexbuf;
		for(j=0;j<maxlen;j++)buf[j] = 0;
	}
	else
	{
		//open
		funcindexfd = open(
			name,
			O_CREAT|O_RDWR|O_BINARY, 
			S_IRWXU|S_IRWXG|S_IRWXO
		);

		//read
		funcindexlen = read(funcindexfd, funcindexbuf, maxlen);
		printf("funcindex:	%x\n", funcindexlen);

		//clean
		buf = (void*)funcindexbuf;
		for(j=funcindexlen;j<maxlen;j++)buf[j] = 0;
	}
}
void funcindex_stop()
{
	lseek(funcindexfd, 0, SEEK_SET);
	write(funcindexfd, funcindexbuf, funcindexlen);
	close(funcindexfd);
}
void funcindex_create()
{
}
void funcindex_delete()
{
}
