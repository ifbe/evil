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
	u32 len;
	u32 off;
	u32 hash0;
	u32 hash1;

	u64 first;
	u64 last;
};
//
static int hashfd;
static int hashlen;
static struct hashtable table[10];
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




int hash_list(u32* hash, struct hashtable** result)
{
	int j,first,last;
	first = 0;
	last = hashlen;
	while(1)
	{
		//
		j = (first+last)/2;
		if(first > last)
		{
			*result = &table[j];
			return 0;
		}

		//
		if(hash[1] < table[j].hash1)
		{
			last = j-1;
		}
		else if(hash[1] > table[j].hash1)
		{
			first = j+1;
		}
		else
		{
			if(hash[0] < table[j].hash0)
			{
				last = j-1;
			}
			else if(hash[0] > table[j].hash0)
			{
				first = j+1;
			}
			else
			{
				*result = &table[j];
				return 1;
			}
		}
	}
}
void hash_choose()
{
}
void hash_read(u8* buf, int len)
{
}
void hash_write(u8* buf, int len)
{
	int j;
	u32 hash[2];
	u8* p;
	struct hashtable* q;
	if(len <= 0)return;

	//
	if(len <= 8)
	{
		p = (u8*)hash;
		for(j=0;j<len;j++)p[j] = buf[j];
		for(j=len;j<8;j++)p[j] = 0;
	}
	else
	{
		hash[1] = bkdrhash(buf, len);
		hash[0] = djb2hash(buf, len);
	}

	j = hash_list(hash, &q);
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
	char* buf = (void*)table;
	for(j=0;j<32*10;j++)buf[j] = 0;

	//char
	charfd = open(
		".42/42.char",
		O_CREAT|O_RDWR|O_TRUNC|O_BINARY,
		S_IRWXU|S_IRWXG|S_IRWXO
	);
	charlen = 0;

	//hash
	hashfd = open(
		".42/42.hash",
		O_CREAT|O_RDWR|O_TRUNC|O_BINARY,
		S_IRWXU|S_IRWXG|S_IRWXO
	);
	hashlen = 0;
}
void hash_delete()
{
	close(hashfd);
	close(charfd);
}