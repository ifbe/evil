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
        //mingw64 compatiable
        #define O_BINARY 0x0
#endif




struct hash
{
	u32 hash0;
	u32 hash1;
	u32 off;
	u32 len;

	u64 first;
	u64 last;
};
//
static int hashfd;
static int hashlen;
static struct hash hbuf[0x8000];
//
static int charfd;
static int charlen;
static u8 sbuf[0x100000];




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
void* hash_search(u32* hash)
{
	int mid;
	int left = 0;
	int right = hashlen;
	u64 xx;
	u64 yy = *(u64*)hash;
	while(1)
	{
		//expand
		if(left >= hashlen)
		{
			if(hashlen >= 0x7fff)return 0;
			return &hbuf[left];
		}

		//lastone
		if(left >= right)
		{
			return &hbuf[left];
		}

		//
		mid = (left+right)/2;
		xx = *(u64*)(&hbuf[mid]);
		if(xx < yy)
		{
			left = mid+1;
		}
		else if(xx > yy)
		{
			right = mid;
		}
		else
		{
			return &hbuf[mid];
		}
	}
}




void hash_read(u8* buf, int len)
{
}
void hash_write(u8* buf, int len)
{
	int j;
	u8* src;
	u8* dst;
	u32 hash[2];
	struct hash* q;


	//1.generate hash
	if(len <= 0)return;
	if(len <= 8)
	{
		dst = (u8*)hash;
		for(j=0;j<len;j++)dst[j] = buf[j];
		for(j=len;j<8;j++)dst[j] = 0;
	}
	else
	{
		hash[1] = bkdrhash(buf, len);
		hash[0] = djb2hash(buf, len);
	}
	//printf("%08x,%08x: %s\n", hash[0], hash[1], buf);


	//no space
	q = hash_search(hash);
	if(q == 0)return;

	//same
	if(*(u64*)q == *(u64*)hash)return;

	if(hashlen > 0x7fff)return;
	hashlen++;

	//move
	dst = (void*)hbuf + hashlen*0x20 + 0x1f;
	src = dst - 0x20;
	while(1)
	{
		*dst = *src;

		dst--;
		src--;
		if(src < (u8*)q)break;
	}

	//insert hash
	q->hash0 = hash[0];
	q->hash1 = hash[1];
	q->off = charlen;
	q->len = len;

	//insert string
	for(j=0;j<len;j++)sbuf[charlen+j] = buf[j];
	charlen += len;
}
void hash_list()
{
}
void hash_choose()
{
}
void hash_start()
{
}
void hash_stop()
{
}
void hash_create()
{
	int j;
	char* buf;

	buf = (void*)hbuf;
	for(j=0;j<0x100000;j++)buf[j] = 0;

	buf = (void*)sbuf;
	for(j=0;j<0x100000;j++)buf[j] = 0;

	//hash
	hashfd = open(
		".42/42.hash",
		O_CREAT|O_RDWR|O_TRUNC|O_BINARY,
		S_IRWXU|S_IRWXG|S_IRWXO
	);
	hashlen = 0;

	//char
	charfd = open(
		".42/42.char",
		O_CREAT|O_RDWR|O_TRUNC|O_BINARY,
		S_IRWXU|S_IRWXG|S_IRWXO
	);
	charlen = 0;
}
void hash_delete()
{
	write(hashfd, hbuf, hashlen*0x20);
	close(hashfd);

	write(charfd, sbuf, charlen);
	close(charfd);
}
