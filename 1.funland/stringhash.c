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
void stringdata_read(int);
int stringdata_write(char*, int);




struct hash
{
	u32 hash0;
	u32 hash1;
	u32 off;
	u32 len;

	u64 first;
	u64 last;
};
static struct hash hashbuf[0x8000];
static int hashfd;
static int hashlen;




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
u64 stringhash_generate(char* buf, int len)
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
void* stringhash_search(u64 hash)
{
	u64 xxx;
	int mid;
	int left = 0;
	int right = hashlen/0x20;
	while(1)
	{
		//expand
		if(left >= hashlen/0x20)
		{
			if(hashlen >= 0xfffe0)return 0;
			return &hashbuf[left];
		}

		//lastone
		if(left >= right)
		{
			return &hashbuf[left];
		}

		//
		mid = (left+right)/2;
		xxx = *(u64*)(&hashbuf[mid]);
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
			return &hashbuf[mid];
		}
	}
}
void stringhash_print(u64 hash)
{
	char buf[9];

	struct hash* h = stringhash_search(hash);
	if(h == 0)return;
	if(*(u64*)h != hash)return;

	if((h->len) > 8)
	{
		stringdata_read(h->off);
	}
	else
	{
		*(u64*)buf = hash;
		buf[8] = 0;
		printf("%-8s\n", buf);
	}
}




void* stringhash_read(u64 hash)
{
	struct hash* h = stringhash_search(hash);
	if(h == 0)return 0;
	if(*(u64*)h != hash)return 0;
	return h;
}
void* stringhash_write(char* buf, int len)
{
	int j;
	struct hash* h;
	u8* src;
	u8* dst;
	u64 temp;

	//
	temp = stringhash_generate(buf, len);

	//no space
	h = stringhash_search(temp);
	if(h == 0)return 0;

	//same
	if(*(u64*)h == temp)return h;

	if(hashlen > 0xfffe0)return 0;
	hashlen += 0x20;

	//move
	dst = (void*)hashbuf + hashlen + 0x1f;
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
	else j = stringdata_write(buf, len);

	//insert hash
	h->hash0 = temp & 0xffffffff;
	h->hash1 = temp >> 32;
	h->off = j;
	h->len = len;
	h->first = 0;
	h->last = 0;

	return h;
}
void stringhash_list()
{
}
void stringhash_choose()
{
}
void stringhash_start(int flag)
{
	int j;
	char* buf;
	char* name = ".42/str.hash";

	if(flag == 0)
	{
		hashfd = open(
			name,
			O_CREAT|O_RDWR|O_TRUNC|O_BINARY,
			S_IRWXU|S_IRWXG|S_IRWXO
		);
		hashlen = 0;

		buf = (void*)hashbuf;
		for(j=0;j<0x100000;j++)buf[j] = 0;
	}
	else
	{
		//open
		hashfd = open(
			name,
			O_CREAT|O_RDWR|O_BINARY,
			S_IRWXU|S_IRWXG|S_IRWXO
		);

		//read
		hashlen = read(hashfd, hashbuf, 0x100000);
		printf("str hash:	%x\n", hashlen);

		//clean
		buf = (void*)hashbuf;
		for(j=hashlen;j<0x100000;j++)buf[j] = 0;
	}
}
void stringhash_stop()
{
}
void stringhash_create()
{
}
void stringhash_delete()
{
	lseek(hashfd, 0, SEEK_SET);
	write(hashfd, hashbuf, hashlen);
	close(hashfd);
}
