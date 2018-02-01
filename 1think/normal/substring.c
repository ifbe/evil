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
#define hex32(a,b,c,d) (a | (b<<8) | (c<<16) | (d<<24))
#ifndef O_BINARY
        #define O_BINARY 0x0
#endif
void relation_write(void* uchip, u64 ufoot, u64 utype, void* bchip, u64 bfoot, u64 btype);
int strhash_export(void*, void*);
void* strhash_write(void*, int);
void* strhash_read(u64);




struct hash
{
	u32 hash0;
	u32 hash1;
	u32 off;
	u32 len;

	u64 first;
	u64 last;
};
static u8* sbuf = 0;
static int scur = 0;




static u64 basehash;
void think_one(u8* buf, int len)
{
	void* baseobj;
	void* thisobj;
	//printf("%.*s\n", len, buf);

	thisobj = strhash_write(buf, len);
	if(thisobj == 0)
	{
		printf("error@7777\n");
		return;
	}

	baseobj = strhash_read(basehash);
	if(baseobj == 0)
	{
		printf("error@6666\n");
		return;
	}

	//hash <- hash
	relation_write(
		baseobj, 0, hex32('h','a','s','h'),
		thisobj, 0, hex32('h','a','s','h')
	);
}
void think_line(u8* buf, int len)
{
	int j,k,val;
	void* baseobj;
	if(buf == 0)return;
	if(len == 0)return;
	//printf("%.*s\n", len, buf);
	//worker_write(buf, len, 4, 0);

	baseobj = strhash_write(buf, len);
	if(baseobj == 0)
	{
		printf("error@6666\n");
		return;
	}
	basehash = *(u64*)baseobj;

	j = 0;
	k = 0;
	while(1)
	{
		if(j >= len)
		{
			if((j != k)&&(j-k < len))
			{
				think_one(buf+k, j-k);
			}
			break;
		}

		val = buf[j];
		if(val >= 0xc0)
		{
			if((j != k)&&(j-k < len))
			{
				think_one(buf+k, j-k);
			}
			k = j;
		}
		else if((val == ' ')|(val == '/')|(val == '_'))
		{
			if((j != k)&&(j-k < len))
			{
				think_one(buf+k, j-k);
			}
			k = j+1;
		}
		else if(val < 0x80)
		{
			if(buf[k] > 0x80)
			{
				think_one(buf+k, j-k);
				k = j;
			}
		}

		j++;
	}
}
void think_part()
{
	int j,k = -1;
	for(j=0;j<scur;j++)
	{
		//printf("%x\n", sbuf[j]);
		if(sbuf[j] == '\n')
		{
			//printf("here\n");
			if((k >= 0) && (j > k))
			{
				//printf("1111\n");
				think_line(sbuf+k, j-k);
				k = -1;
			}
		}
		else if(sbuf[j] >= 0x20)
		{
			if(k < 0)k = j;
		}
		//else printf("%x@%x\n", sbuf[j], j);
	}
}
void think_substr()
{
	u64 haha;

	//malloc
	sbuf = malloc(0x100000);
	if(sbuf == 0)
	{
		printf("error@malloc\n");
		return;
	}

	//
	haha = 0;
	while(1)
	{
		printf("@%llx\n", haha);
		scur = strhash_export(sbuf, &haha);
		if(scur == 0)break;

		think_part();

		haha++;
		if(haha == 0)break;
	}
}
