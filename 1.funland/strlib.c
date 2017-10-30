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
int decstr2data(u8* src, void* dst)
{
	int j;
	u64 data=0;
	for(j=0;j<20;j++)		//64bit的最大数为20个阿拉伯数字
	{
		//1.如果<0x20:		//返回取得的总数量
		if(src[j]<0x30)break;
		if(src[j]>0x39)break;

		//3.如果是正常值:	//先乘10，再加上这个值，然后搞下一个数
		data = data*10;
		data += src[j]-0x30;
	}
	//say("data=%llx\n", data);

	if(data>0xffffffff)*(u64*)dst = data;
	else *(u32*)dst = data&0xffffffff;
	return j;
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
//printf("%c,%c\n",first[j],second[k]);
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