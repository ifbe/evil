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
#define maxlen 0x1000000
static u8 charbuf[maxlen];
static int charfd;
static int charlen;




void stoplearn();
int strdata_write(char* buf, int len)
{
	int j;
/*
	//todo: compress string
	j = search_in_existing_string_memory();
	if(j > 0)return j;
*/
	for(j=0;j<len;j++)charbuf[charlen+j] = buf[j];
	charbuf[charlen+len] = 0xa;

	j = charlen;
	charlen += len+1;
	if(charlen >= maxlen)
	{
		printf("err@strdata\n");
		stoplearn();
		return 0;
	}

	return j;
}
void* strdata_read(int off)
{
	return charbuf + off;
}
void strdata_start(int flag)
{
	int j;
	char* buf;
	char* name = ".42/str/str.txt";

	if(flag == 0)
	{
		charfd = open(
			name,
			O_CREAT|O_RDWR|O_TRUNC|O_BINARY,
			S_IRWXU|S_IRWXG|S_IRWXO
		);
		charlen = 0;

		buf = (void*)charbuf;
		for(j=0;j<maxlen;j++)buf[j] = 0;
	}
	else
	{
		//open
		charfd = open(
			name,
			O_CREAT|O_RDWR|O_BINARY,
			S_IRWXU|S_IRWXG|S_IRWXO
		);

		//read
		charlen = read(charfd, charbuf, maxlen);
		printf("strdata:	%x\n", charlen);

		//clean
		buf = (void*)charbuf;
		for(j=charlen;j<maxlen;j++)buf[j] = 0;
	}
}
void strdata_stop()
{
	lseek(charfd, 0, SEEK_SET);
	write(charfd, charbuf, charlen);
	close(charfd);
}
void strdata_create()
{
}
void strdata_delete()
{
}