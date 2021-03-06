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
#define maxlen 0x100000




static u8 chipindexbuf[maxlen];
static int chipindexfd;
static int chipindexlen;




void* chip_write()
{
	struct chipindex* addr;
	//printf("chip@%d\n", linenum);

	addr = (void*)chipindexbuf + chipindexlen;
	addr->self = chipindexlen;
	addr->type = 0;
	addr->data = 0;

	chipindexlen += 0x20;
	if(chipindexlen >= maxlen)
	{
		printf("err@funcindex\n");
		return 0;
	}
	return addr;
}
void* chip_read(int offset)
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
