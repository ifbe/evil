#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "evil.h"
#ifndef O_BINARY
	#define O_BINARY 0x0
#endif
void strdata_start(int);
void strdata_stop();
void strhash_start(int);
void strhash_stop();
void relation_start(int);
void relation_stop();
//
int bplus_trav(u64*, u64*, u8*, int*);
int strhash_export(void*, void*);
void* strhash_write(void*, int);
void* strhash_read(u64);




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
	relationcreate(baseobj, 0, _hash_, _child_, thisobj, 0, _hash_, _parent_);
}
void think_line(u8* buf, int len)
{
	int j,k,val;
	void* baseobj;
	if(buf == 0)return;
	if(len == 0)return;
	//printf("%.*s\n", len, buf);

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
void think_part(u8* sbuf, int scur)
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
void substr(int argc, char** argv)
{
	int ret;
	u8* sbuf;
	int slen;
	u64 this;
	u64 hash;

	//malloc
	sbuf = malloc(0x100000);
	if(sbuf == 0)
	{
		printf("error@malloc\n");
		return;
	}

	//
	strdata_start(1);
	strhash_start(1);
	relation_start(1);

	//
	this = 0;
	hash = 0;
	while(1)
	{
		ret = bplus_trav(&this, &hash, sbuf, &slen);
		if(0 == ret)break;

		printf("%8llx: %016llx: %.*s\n", this, hash, slen, sbuf);
		//think_part(sbuf, slen);
	}

	//
	relation_stop();
	strhash_stop();
	strdata_stop();
}
