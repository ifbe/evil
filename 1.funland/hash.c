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




struct hashtable
{
	u32 hash1;
	u32 hash2;
	u32 length;
	u32 offset;

	u64 first;
	u64 last;
};
//
static int hashfd;
static int hashlen;
//
static int charfd;
static int charlen;




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




void hash_read(u8* buf, int len)
{
	int j;
	u32 hash = djb2hash(buf, len);
	for(j=0;j<0;j++)
	{}
}
void hash_write(u8* buf, int len)
{
	u32 hash = djb2hash(buf, len);
}
void hash_create()
{
	charfd = open(
		".42/42.char",
		O_CREAT|O_RDWR|O_TRUNC|O_BINARY,
		S_IRWXU|S_IRWXG|S_IRWXO
	);
	hashfd = open(
		".42/42.hash",
		O_CREAT|O_RDWR|O_TRUNC|O_BINARY,
		S_IRWXU|S_IRWXG|S_IRWXO
	);
}
void hash_delete()
{
	close(hashfd);
	close(charfd);
}