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
int hexstr2data(u8* src,u64* data)
{
	int j;
	*data=0;
	for(j=0;j<16;j++)
	{
		//say("%x\n",src[j]);
		//1.如果小于0x20，那就是结束了
		if(src[j]<=0x20) return j;

		//2.如果大于0x80，那就返回错误
		if(src[j]>=0x80) return -1;

		//3.如果是正常值
		*data=(*data)<<4;
		if(src[j]>=0x61 && src[j]<=0x66)
		{
			*data+=src[j]-0x61+10;
		}
		else if(src[j]>=0x30 && src[j]<=0x39)
		{
			*data+=src[j]-0x30;
		}
		else return -2;
	}
	return -999999;	//不会到这里
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
	char* name = ".42/strdata/00";

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
