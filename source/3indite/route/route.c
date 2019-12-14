#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "evil.h"
//
void output(void*, int);
void readthemall(int);
void writethemall(int);
void fixarg(void*, void*);
void strhash_print(u64 hash);
int strhash_export(u64 hash, u8* dst, int len);
u64 strhash_generate(void*, int);
//
void* strhash_read(u64);
void* pin_read(int);
void* chip_read(int);
void* filemd5_read(int);
void* funcindex_read(int);
void* shapeindex_read(int);
void* pointindex_read(int);
//dfs
struct stack
{
	u64 type;
	u64 data;
	void* addr;
	void* padd;
	struct relation* irel;
	struct relation* orel;
};
static struct stack st[16];
static int rsp = 0;
//bfs
struct queue
{
	u64 type;
	u64 addr;
	int parent;
	u8 other[12];
};
static struct queue q0[256];
static int l0 = 0;
static struct queue q1[256];
static int l1 = 0;




void route_aaa(
	struct relation* irel, struct relation* orel,
	u64 type, u64 addr)
{
	int j;

first:
	while(1)
	{
		if(0 == irel)goto second;
		if((type == irel->srctype)&&(addr == irel->srcchip))
		{
			printf("%.4s.%llx.i\n",
				(void*)&irel->dsttype,
				irel->dstchip
			);
			return;
		}
		irel = samedstnextsrc(irel);
	}

second:
	while(1)
	{
		if(0 == orel)goto third;
		if((type == orel->dsttype)&&(addr == orel->dstchip))
		{
			printf("%.4s.%llx.i\n",
				(void*)&orel->srctype,
				orel->srcchip
			);
			return;
		}
		orel = samesrcnextdst(orel);
	}

third:
	return;
}
void route_dfs(u64 t1, struct hash* h1, u64 t2, struct hash* h2)
{
	int j;
	u64 type, data;

	st[0].type = _hash_;
	st[0].data = t1;
	st[0].addr = h1;
	st[0].irel = relationread(h1->irel0);
	st[0].orel = relationread(h2->orel0);
	rsp = 0;
#define maxdepth 8
	//doit
	while(1)
	{
		if(0 != st[rsp].irel)
		{
			type = (st[rsp].irel)->srctype;
			data = (st[rsp].irel)->srcchip;
			st[rsp].irel = samedstnextsrc(st[rsp].irel);
printf("%d.i) %llx,%llx\n",rsp,type,data);

			if((_hash_ == type)&&(t2 == data))break;
			if(rsp >= maxdepth)continue;
			for(j=0;j<=rsp;j++)
			{
				if((type == st[j].type)&&(data == st[j].data))break;
			}
			if(j <= rsp)continue;

			if(_hash_ == type)h1 = strhash_read(data);
			else if(_file_ == type)h1 = filemd5_read(data);
			else if(_func_ == type)h1 = funcindex_read(data);
			else if(_chip_ == type)h1 = chip_read(data);
			else if(_pin_ == type)h1 = pin_read(data);
			else continue;

			rsp++;
			st[rsp].type = type;
			st[rsp].data = data;
			st[rsp].addr = h1;
			st[rsp].irel = relationread(h1->irel0);
			st[rsp].orel = relationread(h1->orel0);
			continue;
		}

		if(0 != st[rsp].orel)
		{
			type = (st[rsp].orel)->dsttype;
			data = (st[rsp].orel)->dstchip;
			st[rsp].orel = samesrcnextdst(st[rsp].orel);
printf("%d.o) %llx,%llx\n",rsp,type,data);

			if((_hash_ == type)&&(t2 == data))break;
			if(rsp >= maxdepth)continue;
			for(j=0;j<=rsp;j++)
			{
				if((type == st[j].type)&&(data == st[j].data))break;
			}
			if(j <= rsp)continue;

			if(_hash_ == type)h1 = strhash_read(data);
			else if(_file_ == type)h1 = filemd5_read(data);
			else if(_func_ == type)h1 = funcindex_read(data);
			else if(_chip_ == type)h1 = chip_read(data);
			else if(_pin_ == type)h1 = pin_read(data);
			else continue;

			rsp++;
			st[rsp].type = type;
			st[rsp].data = data;
			st[rsp].addr = h1;
			st[rsp].irel = relationread(h1->irel0);
			st[rsp].orel = relationread(h1->orel0);
			continue;
		}

		if(0 == rsp)break;
		rsp--;
	}
	for(j=0;j<=rsp;j++)
	{
		if(_hash_ == st[j].type)
		{
			strhash_print(st[j].data);
			printf(" -> ");
		}
		else
		{
			printf("%.4s@%llx -> ", (void*)&st[j].type, st[j].data);
		}
	}
}




int route_add(struct queue* q, int parent, u64 type, u64 addr)
{
	int j,k;
	if(q == q0)k = l0;
	else k = l1;

	for(j=0;j<k;j++)
	{
		if(q[j].type != type)continue;
		if(q[j].addr != addr)continue;
		return j;
	}

	q[k].type = type;
	q[k].addr = addr;
	q[k].parent = parent;

	if(q == q0)l0++;
	else l1++;
	return k;
}
void route_bfs(struct queue* q, int cur, int len)
{
	int j,k;
	struct hash* h;
	struct relation* w;
printf("[%d,%d)\n",cur,len);

	for(j=cur;j<len;j++)
	{
printf("%x:%llx,%llx\n",j,q[j].type, q[j].addr);
		if(_hash_ == q[j].type)
		{
			h = strhash_read(q[j].addr);
			if(h == 0)continue;
		}
		else if(_file_ == q[j].type)
		{
			h = filemd5_read(q[j].addr);
			if(h == 0)continue;
		}
		else if(_func_ == q[j].type)
		{
			h = funcindex_read(q[j].addr);
			if(h == 0)continue;
		}
		else if(_chip_ == q[j].type)
		{
			h = chip_read(q[j].addr);
			if(h == 0)continue;
		}
		else if(_pin_ == q[j].type)
		{
			h = pin_read(q[j].addr);
			if(h == 0)continue;
		}
		else continue;

		w = relationread(h->irel0);
		while(1)
		{
			if(w == 0)break;
			k = route_add(q, j, w->srctype, w->srcchip);
			//if(j != k)graph_pair(j,k);

			w = samedstnextsrc(w);
		}

		w = relationread(h->orel0);
		while(1)
		{
			if(w == 0)break;
			k = route_add(q, j, w->dsttype, w->dstchip);
			//if(j != k)graph_pair(k, j);

			w = samesrcnextdst(w);
		}
	}
}
void route_bibfs(u64 t0, u64 t1)
{
	int i,ret0,ret1;
	int j,k,m,n,x,y;
	char outbuf[0x1000];

	l0 = l1 = 0;
	route_add(q0, 0, _hash_, t0);
	route_add(q1, 0, _hash_, t1);

	ret0 = ret1 = -1;
	j = k = m = n = 0;
	for(i=0;i<4;i++)
	{
		k = l0;
		route_bfs(q0, j, k);
		if(l0 >= 128)break;
		j = k;

		n = l1;
		route_bfs(q1, m, n);
		if(l1 >= 128)break;
		m = n;

		for(y=k;y<l0;y++)
		{
			for(x=n;x<l1;x++)
			{
				if((q0[y].type == q1[x].type)&&(q0[y].addr == q1[x].addr))
				{
					ret0 = y;
					ret1 = x;
					break;
				}
			}
		}
	}
	if(ret0 < 0)return;
	if(ret1 < 0)return;

	for(j=0;j<16;j++)
	{
		if(_hash_ == q0[ret0].type)
		{
			output(outbuf,
			strhash_export(
			q0[ret0].addr, (void*)outbuf, 99));
		}
		else
		{
			printf("%.4s@%llx",
				(void*)&q0[ret0].type, q0[ret0].addr);
		}
		printf(" <- ");

		ret0 = q0[ret0].parent;
		if(0 == ret0)break;
	}
	output(outbuf, strhash_export(q0[0].addr, (void*)outbuf, 99));
	printf("\n");

	for(j=0;j<16;j++)
	{
		if(_hash_ == q1[ret1].type)
		{
			output(outbuf,
			strhash_export(
			q1[ret1].addr, (void*)outbuf, 99));
		}
		else
		{
			printf("%.4s@%llx",
				(void*)&q1[ret1].type, q1[ret1].addr);
		}
		printf(" -> ");

		ret1 = q1[ret1].parent;
		if(0 == ret1)break;
	}
	output(outbuf, strhash_export(q1[0].addr, (void*)outbuf, 99));
	printf("\n");
}




void route(int argc, char** argv)
{
	u64 t1, t2;
	struct hash* h1;
	struct hash* h2;
	//check
	if(argc != 3)
	{
		printf("give me 2 names\n");
		return;
	}
	readthemall(1);

#if (defined(_WIN32) || defined(__WIN32__))
	u8 arg1[0x100];
	u8 arg2[0x100];
	fixarg(arg1, argv[1]);
	fixarg(arg2, argv[2]);
#else
	u8* arg1 = (void*)argv[1];
	u8* arg2 = (void*)argv[2];
#endif

	//str1
	t1 = strhash_generate(arg1, strlen((void*)arg1));
	h1 = strhash_read(t1);
	if(0 == h1)
	{
		printf("(%016llx)%s: notfound\n", t1, arg1);
		return;
	}
	printf("(%016llx)%s: %llx\n", t1, arg1, (u64)h1);

	//str2
	t2 = strhash_generate(arg2, strlen((void*)arg2));
	h2 = strhash_read(t2);
	if(0 == h2)
	{
		printf("(%016llx)%s: notfound\n", t2, arg2);
		return;
	}
	printf("(%016llx)%s: %llx\n", t2, arg2, (u64)h2);

	//route_dfs(t1, h1, t2, h2);
	route_bibfs(t1, t2);
}
