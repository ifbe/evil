#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
int worker_write(void* buf, int len, int type, int haha);



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
