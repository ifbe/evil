#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define hex32(a,b,c,d) (a | (b<<8) | (c<<16) | (d<<24))
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
void* strhash_write(void* buf, int len);



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
					strhash_write(buf+j, k-j);
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
void utf8_start()
{
}
void utf8_stop()
{
}




void utf8_create(u64* that, u64* this)
{
	this[0] = 0x6573726170;
	this[1] = hex32('u','t','f','8');
	this[4] = (u64)utf8_start;
	this[5] = (u64)utf8_stop;
	this[6] = (u64)utf8_read;
	this[7] = (u64)utf8_write;
}
