#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#define hex32(a,b,c,d) (a | (b<<8) | (c<<16) | (d<<24))
struct vertex
{
	float x;
	float y;
	float z;
	float w;
};
struct binfo
{
        u64 vertexcount;
        u64 normalcount;
        u64 colorcount;
        u64 texturecount;
        u64 pointcount;
        u64 linecount;
        u64 tricount;
        u64 rectcount;
};




void graph_init(void* vertexbuf, struct binfo* info)
{
}
void graph_data(void* vertexbuf, u64* info)
{
	int j;
	for(j=0;j<8;j++)printf("%08llx ", info[j]);
	printf("\n");
}
