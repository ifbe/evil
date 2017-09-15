#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#ifndef O_BINARY
        #define O_BINARY 0x0
#endif
void* hash_read(u64 hash);



struct hash
{
	u32 hash0;
	u32 hash1;
	u32 off;
	u32 len;

	u64 first;
	u64 last;
};
struct wire
{
	u32 pintype;
	u32 detail;
	u32 samepinprevchip;
	u32 samepinnextchip;

	u32 chipinfo;
	u32 footinfo;
	u32 samechipprevpin;
	u32 samechipnextpin;
};
static u8 wirebuf[0x100000];
static int wirefd;
static int wirelen;
u64 parsetype(int rel, int off)
{
	//printf("rel=%d,off=%d\n",rel,off);
#define hex32(a,b,c,d) (a | (b<<8) | (c<<16) | (d<<24))
	if(rel == 1)
	{
		if(off == 0)return hex32('F','I','L','E');
		else if(off == 1)return hex32('f','i','l','e');
	}
	else if(rel == 2)
	{
		if(off == 0)return hex32('F','U','N','C');
		else if(off == 1)return hex32('f','u','n','c');
	}
}




//'dir'		xxxxxxx,	dirname,	filename
//'file',	linenum,	filename,	funcname
//'call',	linenum,	funcname,	callname
//'struct',	linenum,	structname,	elementname
void connect_write(u64 upper, u64 below, u64 relation, u64 detail)
{
	u64 tmp;
	struct hash* h1;
	struct hash* h2;
	struct wire* w1;
	struct wire* w2;
	struct wire* wc;
	//printf("%llx, %llx\n", upper, below);


	//1. current location
	h1 = hash_read(upper);
	h2 = hash_read(below);
	if( (h1 == 0)|(h2 == 0) )
	{
		printf("error@connect_write\n");
		return;
	}


	//2. upper pin
	w1 = (void*)wirebuf + (h1->first);
	if(h1->first == 0)
	{
		//printf("@2\n");
		if(wirelen >= 0xfffc0)return;

		w1 = (void*)wirebuf + wirelen;
		wirelen += 0x20;

		w1->pintype = parsetype(relation, 0);
		w1->detail = detail;

		w1->chipinfo = upper&0xffffffff;
		w1->footinfo = (upper>>32);

		h1->first = (void*)w1 - (void*)wirebuf;
		//printf("first=%llx\n",h1->first);
	}


	//3. below pin
	if(wirelen >= 0xfffc0)return;

	w2 = (void*)wirebuf + wirelen;
	wirelen += 0x20;

	w2->pintype = parsetype(relation, 1);
	w2->detail = detail;

	w2->chipinfo = below&0xffffffff;
	w2->footinfo = (below>>32);


	//4. relation
	//w1->samepinprevchip = 0;		//certainly
	//w1->samechipprevpin = ?;		//unchanged
	//w1->samechipnextpin = ?;		//unchanged

	w2->samechipprevpin = 0;
	w2->samechipnextpin = 0;

	if(w1->samepinnextchip == 0)
	{
		w1->samepinnextchip = (void*)w2 - (void*)wirebuf;

		w2->samepinprevchip = (void*)w1 - (void*)wirebuf;
		w2->samepinnextchip = 0;		//certainly
	}
	else
	{
		wc = w1;
		while(wc->samepinnextchip != 0)
		{
			wc = (void*)wirebuf + (wc->samepinnextchip);
		}
		wc->samepinnextchip = (void*)w2 - (void*)wirebuf;

		w2->samepinprevchip = (void*)wc - (void*)wirebuf;
		w2->samepinnextchip = 0;		//certainly
	}
}
void connect_read()
{
}
void connect_list()
{
}
void connect_choose()
{
}
void connect_start()
{
	lseek(wirefd, 0, SEEK_SET);
	wirelen = 0x20;
}
void connect_stop()
{
}
void connect_create()
{
	int j;
	char* buf;

	buf = (void*)wirebuf;
	for(j=0;j<0x100000;j++)buf[j] = 0;

	//wire
	wirefd = open(
		".42/42.wire",
		O_CREAT|O_RDWR|O_BINARY,	//O_CREAT|O_RDWR|O_TRUNC|O_BINARY,
		S_IRWXU|S_IRWXG|S_IRWXO
	);

	//
	wirelen = read(wirefd, wirebuf, 0x100000);
	printf("wire:	%x\n", wirelen);

	connect_start();
}
void connect_delete()
{
	write(wirefd, wirebuf, wirelen);
	close(wirefd);
}