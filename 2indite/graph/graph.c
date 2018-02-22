#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#define hex32(a,b,c,d) (a | (b<<8) | (c<<16) | (d<<24))
#define hex64(a,b,c,d,e,f,g,h) (hex32(a,b,c,d) | (((u64)hex32(e,f,g,h))<<32))
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
void graph_init(void*, void*, void*, int);
void graph_data(void*, void*, void*, int);
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




struct pointindex
{
	u64 self;
	u64 ofst;

	u64 irel;
	u64 orel;
};
struct shapeindex
{
	u64 self;
	u64 type;
	u64 irel;
	u64 orel;
};
struct hash
{
	u32 hash0;
	u32 hash1;
	u32 off;
	u32 len;

	u64 irel;
	u64 orel;
};
struct wire
{
	u64 destchip;
	u64 destfoot;
	u32 desttype;		//eg: 'hash', 'dir', 'file', 'func'
	u32 destflag;
	u32 samepinprevchip;
	u32 samepinnextchip;

	u64 selfchip;
	u64 selffoot;
	u32 selftype;		//eg: 'dir', 'file', 'func', 'hash'
	u32 selfflag;
	u32 samechipprevpin;
	u32 samechipnextpin;
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
static struct binfo info;
static u8 buffer[0x800000];
//
struct context
{
	u64 type;
	u64 addr;
	u8 str[16];
};
static int ctxlen;
static struct context ctxbuf[0x1000];




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
	u32* addr;

	if(j==k)return 0;
	j = 2*j;
	k = 2*k+1;
	data = (k<<16)+j;

	addr = (void*)buffer + 0x500000;
	for(i=0;i<info.linecount;i++)
	{
		if(addr[i] == data)return 0;
	}

	addr[info.linecount] = data;
	printf("%x)%x,%x\n", info.linecount, j, k);

	info.linecount += 1;
	return 1;
}
void graph_bfs(int cur, int len)
{
	int j,k;
	struct hash* h;
	struct wire* w;
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
	float* vv;
	float* nn;
	float* cc;

	p = (void*)&info;
	for(j=0;j<16;j++)p[j] = 0;

	temp = strhash_generate(buf, len);

	ctxlen = 0;
	graph_add(_hash_, temp);

	j = 0;
	for(i=0;i<20;i++)
	{
		m = ctxlen;
		n = info.linecount;

//printf("before=%d\n",info.linecount);
		graph_bfs(j, ctxlen);
//printf("after=%d\n",info.linecount);

		if(info.linecount >= 0x1000)
		{
			ctxlen = m;
			info.linecount = n;
			break;
		}
		if(ctxlen <= m)break;
		if(ctxlen >= 1000)break;

		j = m;
	}

	for(j=0;j<ctxlen;j++)
	{
		vv = (void*)buffer + 0x000000 + 24*j;
		vv[0] = (float)(rand()&0xffff)/65536.0;
		vv[1] = (float)(rand()&0xffff)/65536.0;
		vv[2] = (float)(rand()&0xffff)/65536.0;
		//vv[3] = vv[0];
		//vv[4] = vv[1];
		//vv[5] = vv[2];

		nn = (void*)buffer + 0x100000 + 24*j;
		nn[0] = 0.0;
		nn[1] = 0.0;
		nn[2] = 1.0;
		nn[3] = 0.0;
		nn[4] = 0.0;
		nn[5] = 1.0;

		cc = (void*)buffer + 0x200000 + 24*j;
		if(_hash_ == ctxbuf[j].type){cc[0]=1.0;cc[1]=0.0;cc[2]=0.0;}
		else if(_file_ == ctxbuf[j].type){cc[0]=0.0;cc[1]=1.0;cc[2]=0.0;}
		else if(_func_ == ctxbuf[j].type){cc[0]=0.0;cc[1]=0.0;cc[2]=1.0;}
		else if(_chip_ == ctxbuf[j].type){cc[0]=0.4;cc[1]=1.0;cc[2]=0.7;}
		else if( _pin_ == ctxbuf[j].type){cc[0]=0.8;cc[1]=0.3;cc[2]=1.0;}
		else {cc[0] = 1.0;cc[1] = 1.0;cc[2] = 1.0;}

		cc[3] = 0.1;cc[4] = 0.1;cc[5] = 0.1;
	}
	graph_data(buffer, &info, ctxbuf, ctxlen);
}
void graph(int argc, char** argv)
{
	char buf[0x1000];
	char* p;
	int j;

	readthemall(1);

	graph_init(buffer, &info, ctxbuf, ctxlen);
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
