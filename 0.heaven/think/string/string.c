#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
int worker_write(char* buf, int len, int type, int haha);




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
void string_split(char* buf, int len)
{
	int j,k,val,flag;
	if(buf == 0)return;
	if(len == 0)
	{
		while(buf[len] != 0)len++;
	}
	worker_write(buf, len, 4, 0);

	j = 0;
	k = -1;
	while(1)
	{
		if(j >= len)val = 0;
		else val = buf[j];

		if( ( (val >= '0')&&(val <= '9') ) |
			( (val >= 'A')&&(val <= 'Z') ) |
			( (val >= 'a')&&(val <= 'z') ) |
			(val == '.'))
		{
			if(k<0)k = j;
			flag = 0;
		}
		else flag = 1;

		if( (flag == 1) && (j-k < len) )
		{
			worker_write(buf+k, j-k, 3, 0);
			//printf("%.*s\n", j-k, buf+k);
			k = -1;
		}

		j++;
		if(j > len)break;
	}
}