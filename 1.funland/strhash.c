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
void strdata_read(int, int);
int strdata_write(char*, int);




struct hash
{
	u32 hash0;
	u32 hash1;
	u32 off;
	u32 len;

	u64 first;
	u64 last;
};
#define maxlen 0x100000
static u8* hashbuf[256];
static int hashfd[256];
static int hashlen[256];




u32 bkdrhash(char* buf, int len)
{
	int j;
	u32 hash = 0;
	for(j=0;j<len;j++)
	{
		hash = hash * 131 + buf[j];
	}
	return hash;
}
u32 djb2hash(char* buf, int len)
{
	int j;
	u32 hash=5381;
	for(j=0;j<len;j++)
	{
		hash=(hash<<5) + hash + buf[j];
	}
	return hash;
}
u64 strhash_generate(char* buf, int len)
{
	int j;
	u32 this[2];
	u8* dst;

	if(len <= 8)
	{
		dst = (u8*)this;
		for(j=0;j<len;j++)dst[j] = buf[j];
		for(j=len;j<8;j++)dst[j] = 0;
	}
	else
	{
		this[1] = bkdrhash(buf, len);
		this[0] = djb2hash(buf, len);
	}
	//printf("%08x,%08x: %s\n", this[0], this[1], buf);

	return *(u64*)this;
}




void* strhash_search(u64 hash)
{
	u64 xxx;
	int index, mid, left, right;
	index = (hash >> 56);// & 0xf0;

	left = 0;
	right = hashlen[index]/0x20;
	while(1)
	{
		//lastone
		if(left >= right)
		{
			return hashbuf[index] + (left*0x20);
		}

		//
		mid = (left+right)/2;
		xxx = *(u64*)(hashbuf[index] + (mid*0x20));
		if(xxx < hash)
		{
			left = mid+1;
		}
		else if(xxx > hash)
		{
			right = mid;
		}
		else
		{
			return hashbuf[index] + (mid*0x20);
		}
	}
}
void strhash_print(u64 hash)
{
	struct hash* h = strhash_search(hash);
	if(h == 0)return;
	if(*(u64*)h != hash)return;

	if((h->len) > 8)
	{
		strdata_read(h->off, h->len);
	}
	else
	{
		printf("%-8.8s\n", (char*)h);
	}
}




void stoplearn();
void* strhash_write(char* buf, int len)
{
	int j;
	int index;
	struct hash* h;
	u8* src;
	u8* dst;
	u64 temp;

	//
	temp = strhash_generate(buf, len);

	//notfound
	h = strhash_search(temp);
	if(h == 0)return 0;

	//repeat
	if(*(u64*)h == temp)return h;

	//
	index = (temp >> 56);// & 0xf0;
	hashlen[index] += 0x20;
	if(	((index == 0) && (hashlen[index] >= 0x1000000 - 0x20))
	|	((index != 0) && (hashlen[index] >= maxlen - 0x20)) )
	{
		printf("err@strhash");
		stoplearn();
		return 0;
	}

	//move
	dst = hashbuf[index] + hashlen[index] + 0x1f;
	src = dst - 0x20;
	while(1)
	{
		*dst = *src;

		dst--;
		src--;
		if(src < (u8*)h)break;
	}

	//insert string
	if(len <= 8)j = 0;
	else j = strdata_write(buf, len);

	//insert hash
	h->hash0 = temp & 0xffffffff;
	h->hash1 = temp >> 32;
	h->off = j;
	h->len = len;
	h->first = 0;
	h->last = 0;

	//printf("%d	%s\n", j, buf);
	return h;
}
void* strhash_read(u64 hash)
{
	struct hash* h = strhash_search(hash);
	if(h == 0)return 0;
	if(*(u64*)h != hash)return 0;
	return h;
}
void strhash_list()
{
}
void strhash_choose()
{
}
void strhash_start(int type)
{
	int j, k;
	int flag1, flag2;
	char name[32];
	u8* p;
	if(type != 0)printf("strhash:\n");

	for(j=0;j<256;j++)
	{
		//name
		snprintf(name, 32, ".42/strhash/%02x", j);
		flag1 = O_CREAT|O_RDWR|O_BINARY;
		if(type == 0)flag1 |= O_TRUNC;
		flag2 = S_IRWXU|S_IRWXG|S_IRWXO;

		//open
		hashfd[j] = open(name, flag1, flag2);
		if(hashfd[j] <= 0)
		{
			printf("error@hashfd%02x\n", j);
			exit(-1);
		}

		//malloc
		if(j == 0)p = malloc(0x1000000);
		else p = malloc(maxlen);
		hashbuf[j] = p;

		//read
		if(type == 0)k = 0;
		else
		{
			k = read(hashfd[j], p, maxlen);
			//printf("	%02x:	%x\n", j, k);
		}
		hashlen[j] = k;

		//clean
		for(;k<maxlen;k++)p[k] = 0;
	}
}
void strhash_stop()
{
	int j;
	for(j=0;j<256;j++)
	{
		lseek(hashfd[j], 0, SEEK_SET);
		write(hashfd[j], hashbuf[j], hashlen[j]);
		close(hashfd[j]);
	}
}
void strhash_create()
{
}
void strhash_delete()
{
}
