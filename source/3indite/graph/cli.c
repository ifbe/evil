#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#define hex32(a,b,c,d) (a | (b<<8) | (c<<16) | (d<<24))
static void* ctxbuf;
static int ctxlen;




void graph_init(void* cb, int cl, void* lb, int ll)
{
	ctxbuf = cb;
	ctxlen = cl;
}
void graph_data(void* cb, int cl, void* lb, int ll)
{
}
