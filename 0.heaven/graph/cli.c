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
};




void graph_init()
{
}
void graph_data(
	struct vertex* vertexbuf, int vertexlen,
	struct vertex* normalbuf, int normallen,
	struct vertex* colourbuf, int colourlen,
	u16* rectbuf, int rectlen,
	u16* tribuf, int trilen,
	u16* linebuf, int linelen,
	u16* pointbuf, int pointlen)
{
	printf("%d, %d, %d, %d, %d\n", vertexlen, rectlen, trilen, linelen, pointlen);
	printf("%f, %f, %f\n", vertexbuf[1].x, vertexbuf[1].y, vertexbuf[1].z);
}