#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#ifndef O_BINARY
        #define O_BINARY 0x0
#endif




//
static int charfd;
static int charlen;
static u8 charbuf[0x100000];




char* eatdir(char* p)
{
	int j=-1;
	int k=0;
	while(1)
	{
		if(p[k]==0x0)break;
		if(p[k]=='/')j=k+1;

		k++;
	}

	if(j<0)return 0;
	else return p+j;
}
char* suffix_string(char* p)
{
	int j=-1;
	int k=0;
	while(1)
	{
		if(p[k]==0x0)break;
		else if(p[k]=='/')j=-1;
		else if(p[k]=='.')j=k+1;

		k++;
	}

	if(j<0)return 0;
	else return p+j;
}
u64 suffix_value(char* p)
{
	u8 ret[8];
	int j;

	char* x=suffix_string(p);
	if(x==0)return 0;
	//printf("%s\n",x);

	*(u64*)ret = 0;
	for(j=0;j<8;j++)
	{
		if(x[j] == 0)break;
		ret[j]=x[j];
	}

	return *(u64*)ret;
}
int match(char* first,char* second)
{
	int j=0;
	int k=0;
	while(1)
	{
		if( (first[j]==0) && (second[k]==0) )
		{
			break;
		}
printf("%c,%c\n",first[j],second[k]);
		if(first[j]=='*')
		{
			j++;
			if(first[j]==0)return 1;
		}
		else if(second[k]=='*')
		{
			k++;
			if(second[k]==0)return 1;
		}
		else if(
			(first[j]=='?') |
			(second[k]=='?') |
			(first[j]==second[k]) )
		{
			j++;
			k++;
		}
		else return 0;
	}

	return 1;
}




int string_read(char* buf, int off, int len)
{
	//
}
int string_write(char* buf, int len)
{
	int j;
	for(j=0;j<len;j++)charbuf[charlen+j] = buf[j];
	charlen += len;
	return charlen;
}
void string_create()
{
	int j;
	char* buf;

	buf = (void*)charbuf;
	for(j=0;j<0x100000;j++)buf[j] = 0;

	//char
	charfd = open(
		".42/42.char",
		O_CREAT|O_RDWR|O_TRUNC|O_BINARY,
		S_IRWXU|S_IRWXG|S_IRWXO
	);
	charlen = 0;
}
void string_delete()
{
	write(charfd, charbuf, charlen);
	close(charfd);
}
