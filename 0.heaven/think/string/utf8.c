#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
int worker_write(char* buf, int len, int type, int haha);




void utf8_read_one(u8* buf, int len)
{
	int j=0,k;
	while(1)
	{
		if(buf[j] < 0x80)k=1;
		else if(buf[j] > 0xfc)k=6;
		else if(buf[j] > 0xf8)k=5;
		else if(buf[j] > 0xf0)k=4;
		else if(buf[j] > 0xe0)k=3;
		else if(buf[j] > 0xc0)k=2;
		else break;

		//printf("%.*s\n", k, buf+j);
		worker_write(buf+j, k, 3, 0);

		j += k;
		if(j >= len)break;
	}
}
void utf8_read(u8* buf, int len)
{
	int j=0;	//line head
	int k=0;	//line tail
	int t;
	while(1)
	{
		if( (buf[k] == 0xd) | (buf[k] == 0xa) )
		{
			if(j < k)
			{
				if(buf[j] == '#')
				{
				}
				else
				{
					//printf("%.*s\n", k-j, buf+j);
					worker_write(buf+j, k-j, 4, 0);
					utf8_read_one(buf+j, k-j);
				}
			}
			j = k+1;
		}

		k++;
		if(k >= len)break;
	}
}
void utf8_write()
{
}
void utf8_list()
{
}
void utf8_choose()
{
}
void utf8_start()
{
}
void utf8_stop()
{
}
void utf8_create(u64* that, u64* this)
{
	this[0] = 0x6573726170;
	this[1] = 0x38667475;
	this[2] = (u64)utf8_start;
	this[3] = (u64)utf8_stop;
	this[4] = (u64)utf8_list;
	this[5] = (u64)utf8_choose;
	this[6] = (u64)utf8_read;
	this[7] = (u64)utf8_write;
}