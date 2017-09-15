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
int string_write(char*, int);




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




u32 bkdrhash(u8* buf, int len)
{
	int j;
	u32 hash = 0;
	for(j=0;j<len;j++)
	{
		hash = hash * 131 + buf[j];
	}
	return hash;
}
u32 djb2hash(u8* buf, int len)
{
	int j;
	u32 hash=5381;
	for(j=0;j<len;j++)
	{
		hash=(hash<<5) + hash + buf[j];
	}
	return hash;
}
void* hash_search(u64 hash)
{
	u64 xxx;
	int mid;
	int left = 0;
	int right = hashlen;
	while(1)
	{
		//expand
		if(left >= hashlen)
		{
			if(hashlen >= 0x7fff)return 0;
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




void* hash_read(u64 hash)
{
	struct hash* h = hash_search(hash);
	if(h == 0)return 0;
	if(*(u64*)h != hash)return 0;
	return h;
}
void* hash_write(u8* buf, int len)
{
	int j;
	u8* src;
	u8* dst;
	u64 temp;
	u32 this[2];
	struct hash* h;

	//1.generate hash
	if(len <= 0)return 0;
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

	//
	temp = *(u64*)this;

	//no space
	h = hash_search(temp);
	if(h == 0)return 0;

	//same
	if(*(u64*)h == temp)return h;

	if(hashlen > 0x7fff)return 0;
	hashlen++;

	//move
	dst = (void*)hashbuf + hashlen*0x20 + 0x1f;
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
	else j = string_write(buf, len);

	//insert hash
	h->hash0 = this[0];
	h->hash1 = this[1];
	h->off = j;
	h->len = len;
	h->first = 0;
	h->last = 0;

	return h;
}
void hash_list()
{
}
void hash_choose()
{
}
void hash_start()
{
	lseek(hashfd, 0, SEEK_SET);
	hashlen = 0;
}
void hash_stop()
{
}
void hash_create()
{
	int j;
	char* buf;

	buf = (void*)hashbuf;
	for(j=0;j<0x100000;j++)buf[j] = 0;

	//hash
	hashfd = open(
		".42/42.hash",
		O_CREAT|O_RDWR|O_BINARY,	//O_CREAT|O_RDWR|O_TRUNC|O_BINARY,
		S_IRWXU|S_IRWXG|S_IRWXO
	);

	//
	hashlen = read(hashfd, hashbuf, 0x100000);
	printf("hash:	%x\n", hashlen);

	hash_start();
}
void hash_delete()
{
	write(hashfd, hashbuf, hashlen*0x20);
	close(hashfd);
}
