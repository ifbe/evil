#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "evil.h"
void* chip_write();
void* pin_write();
void* strhash_read(u64);
void* strhash_write(u8*, int);




static int count = 0;
static int infunc = 0;
static void* chip;




static void map_read_line(u8* buf, int len)
{
	int j;
	void* addr;
	//printf("%.*s\n", len, buf);

	if('}' == buf[len-1])
	{
		infunc--;
		return;
	}

	j = infunc;
	if('{' == buf[len-1])
	{
		infunc++;
		len--;
	}
	if(0 == len)return;

	addr = strhash_write(buf, len);
	if(0 == j)
	{
		chip = chip_write();
		relationcreate(addr, count, _hash_, chip, 0, _chip_);
		count = 0;
	}
	else
	{
		relationcreate(chip, count, _chip_, addr, 0, _hash_);
		count++;
	}
}
static void map_read(u8* buf, int len)
{
	int j,k,m,n;
	k = 0;
	for(j=0;j<len;j++)
	{
		if(buf[j] == 0xa)
		{
			for(m=k;m<j;m++)if(buf[m] > 0x20)break;
			for(n=j-1;n>=m;n--)if(buf[n] > 0x20)break;
			if(n >= m)map_read_line(buf+m, n-m+1);
			k = j+1;
		}
	}
}
static void map_write()
{
}
static void map_list()
{
}
static void map_choose()
{
}
static void map_stop()
{
}
static void map_start()
{
	infunc = 0;
}
void map_delete()
{
}
void map_create(u64* that, u64* this)
{
	this[0] = 0x6573726170;
	this[1] = 0x70616d;
	this[2] = (u64)map_start;
	this[3] = (u64)map_stop;
	this[4] = (u64)map_list;
	this[5] = (u64)map_choose;
	this[6] = (u64)map_read;
	this[7] = (u64)map_write;
}

