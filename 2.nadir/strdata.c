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
static u8* charbuf;
static int charfd;
static int charcur;
static int charlen;




void stoplearn();
int strdata_write(char* buf, int len)
{
	int j;
	if(charcur >= charlen-0x100)
	{
		lseek(charfd, 0, SEEK_SET);
		write(charfd, charbuf, charcur);

		charlen *= 2;
		charbuf = realloc(charbuf, charlen);
		printf("charbuf realloc: %x/%x\n", charbuf, charlen);
	}

	for(j=0;j<len;j++)charbuf[charcur+j] = buf[j];
	charbuf[charcur+len] = 0xa;

	j = charcur;
	charcur += len+1;

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

		charcur = 0;
		charlen = 0x100000;
		charbuf = malloc(charlen);

		buf = (void*)charbuf;
		for(j=0;j<charlen;j++)buf[j] = 0;
	}
	else
	{
		//open
		charfd = open(
			name,
			O_CREAT|O_RDWR|O_BINARY,
			S_IRWXU|S_IRWXG|S_IRWXO
		);

		//malloc
		charcur = 0;
		charlen = 0x100000;
		charbuf = malloc(charlen);

		//read
		while(1)
		{
			j = read(charfd, charbuf+charcur, charlen-charcur);
			if(j != charlen-charcur)
			{
				charcur += j;
				break;
			}

			charcur = charlen;
			charlen *= 2;
			charbuf = realloc(charbuf, charlen);
		}
		printf("strdata:	%x\n", charcur);
		for(j=charcur;j<charlen;j++)charbuf[j] = 0;
	}
}
void strdata_stop()
{
	lseek(charfd, 0, SEEK_SET);
	write(charfd, charbuf, charcur);
	close(charfd);
}
void strdata_create()
{
}
void strdata_delete()
{
}
