#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "evil.h"
#define _hash_ hex32('h','a','s','h')
#define _file_ hex32('f','i','l','e')
#define _func_ hex32('f','u','n','c')
#define _chip_ hex32('c','h','i','p')
#define _pin_ hex32('p','i','n',0)
#define _shape_ hex32('s','h','a','p')
#define _point_ hex32('p','o','i','n')
#define _line_ hex32('l','i','n','e')
#define _tri_ hex32('t','r','i',0)
#define _rect_ hex32('r','e','c','t')
void readthemall(int);
void trianglenormal(void* n, void* a, void* b, void* c);
//
void graph_init(void*, int, void*, int);
void graph_data(void*, int, void*, int);
//
int strhash_export(u64 hash, u8* dst, int len);
u64 strhash_generate(void*, int);
void* strhash_read(u64);
void* pin_read(int);
void* chip_read(int);
void* shapeindex_read(int);
void* pointindex_read(int);
void* pointdata_read(int);
void* funcindex_read(int);
void* filemd5_read(int);
//
void* samepinprevchip(void*);
void* samepinnextchip(void*);
void* samechipprevpin(void*);
void* samechipnextpin(void*);
void* relation_read(int);




struct context
{
	u64 type;
	u64 addr;
	u8 str[16];
};
static struct context ctxbuf[0x100000/0x20];
static int ctxlen = 0;
static u32 linebuf[0x100000/4];
static int linelen = 0;




int graph_add(u64 type, u64 addr)
{
	int j,k;

	k = ctxlen;
	for(j=0;j<k;j++)
	{
		if(ctxbuf[j].type != type)continue;
		if(ctxbuf[j].addr != addr)continue;
		return j;
	}

	ctxbuf[k].type = type;
	ctxbuf[k].addr = addr;
	if(type == _hash_)
	{
		strhash_export(addr, ctxbuf[k].str, 16);
	}

	ctxlen++;
	return k;
}
int graph_pair(int j, int k)
{
	int i;
	u32 data;

	if(j==k)return 0;
	j = 2*j;
	k = 2*k+1;
	data = (k<<16)+j;

	for(i=0;i<linelen;i++)
	{
		if(linebuf[i] == data)return 0;
	}

	linebuf[linelen] = data;
	printf("%x)%x,%x\n", linelen, j, k);

	linelen += 1;
	return 1;
}
void graph_bfs(int cur, int len)
{
	int j,k;
	struct hash* h;
	struct relation* w;
printf("[%d,%d)\n",cur,len);

	for(j=cur;j<len;j++)
	{
printf("%x:%llx,%llx\n",j,ctxbuf[j].type, ctxbuf[j].addr);
		if(_hash_ == ctxbuf[j].type)
		{
			h = strhash_read(ctxbuf[j].addr);
			if(h == 0)continue;
		}
		else if(_file_ == ctxbuf[j].type)
		{
			h = filemd5_read(ctxbuf[j].addr);
			if(h == 0)continue;
		}
		else if(_func_ == ctxbuf[j].type)
		{
			h = funcindex_read(ctxbuf[j].addr);
			if(h == 0)continue;
		}
		else if(_chip_ == ctxbuf[j].type)
		{
			h = chip_read(ctxbuf[j].addr);
			if(h == 0)continue;
		}
		else if(_pin_ == ctxbuf[j].type)
		{
			h = pin_read(ctxbuf[j].addr);
			if(h == 0)continue;
		}
		else continue;

		w = relation_read(h->irel);
		while(1)
		{
			if(w == 0)break;
			k = graph_add(w->selftype, w->selfchip);
			if(j != k)graph_pair(j,k);

			w = samepinprevchip(w);
		}

		w = relation_read(h->orel);
		while(1)
		{
			if(w == 0)break;
			k = graph_add(w->desttype, w->destchip);
			if(j != k)graph_pair(k, j);

			w = samechipprevpin(w);
		}
	}

}
void graph_one(char* buf, int len)
{
	int i,j,m,n;
	u64 temp;
	u32* p;

	linelen = 0;
	temp = strhash_generate(buf, len);

	ctxlen = 0;
	graph_add(_hash_, temp);

	j = 0;
	for(i=0;i<20;i++)
	{
		m = ctxlen;
		n = linelen;

//printf("before=%d\n", linelen);
		graph_bfs(j, ctxlen);
//printf("after=%d\n", linelen);

		if(linelen >= 0x1000)
		{
			ctxlen = m;
			linelen = n;
			break;
		}
		if(ctxlen <= m)break;
		if(ctxlen >= 1000)break;

		j = m;
	}

	graph_data(ctxbuf, ctxlen, linebuf, linelen);
}
void graph(int argc, char** argv)
{
	char buf[0x1000];
	char* p;
	int j;

	readthemall(1);

	graph_init(ctxbuf, ctxlen, linebuf, linelen);
	for(j=1;j<argc;j++)
	{
		graph_one(argv[j], strlen(argv[j]));
	}
	while(1)
	{
		printf("->");
		fgets(buf, 0x1000, stdin);
		for(j=0;j<0x1000;j++)
		{
			if(buf[j] <= 0xa)
			{
				buf[j] = 0;
				break;
			}
		}
		if((buf[0] == 'q')&&(buf[1] == 0))break;

		j = strlen(buf);
		if(j != 0)graph_one(buf, j);
	}
}
