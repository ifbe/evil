#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#define hex32(a,b,c,d) (a | (b<<8) | (c<<16) | (d<<24))
#define hex64(a,b,c,d,e,f,g,h) (hex32(a,b,c,d) | (((u64)hex32(e,f,g,h))<<32))
#define __hash__ hex32('h','a','s','h')
#define __chip__ hex32('c','h','i','p')
void* chip_write();
void* pin_write();
void* strhash_read(u64);
void* strhash_write(u8*, int);
//
void* samepinprevchip(void*);
void* samepinnextchip(void*);
void* samechipprevpin(void*);
void* samechipnextpin(void*);
void* relation_read(int);
void relation_write(
	void* uchip, u64 ufoot, u64 utype,
	void* bchip, u64 bfoot, u64 btype);




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
		relation_write(
			addr, count, __hash__,
			chip, 0, __chip__
		);
		count = 0;
	}
	else
	{
		relation_write(
			chip, count, __chip__,
			addr, 0, __hash__
		);
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

