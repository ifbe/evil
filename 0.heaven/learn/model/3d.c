#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
void* strhash_write(void* buf, int len);




void three_read(u8* buf, int len)
{
	printf(".3d\n");
}
void three_write()
{
}
void three_list()
{
}
void three_choose()
{
}
void three_start()
{
}
void three_stop()
{
}
void three_create(u64* that, u64* this)
{
	this[0] = 0x6573726170;
	this[1] = 0x6433;
	this[2] = (u64)three_start;
	this[3] = (u64)three_stop;
	this[4] = (u64)three_list;
	this[5] = (u64)three_choose;
	this[6] = (u64)three_read;
	this[7] = (u64)three_write;
}
