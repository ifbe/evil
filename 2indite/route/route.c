#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#define hex32(a,b,c,d) (a | (b<<8) | (c<<16) | (d<<24))
#define hex64(a,b,c,d,e,f,g,h) (hex32(a,b,c,d) | (((u64)hex32(e,f,g,h))<<32))
#define __hash__ hex32('h','a','s','h')
#define __file__ hex32('f','i','l','e')
#define __fun__ hex32('f','u','n','c')
#define __chip__ hex32('c','h','i','p')
#define __pin__ hex32('p','i','n',0)
//
void readthemall(int);
void strhash_print(u64 hash);
int strhash_export(u64 hash, u8* dst, int len);
u64 strhash_generate(void*, int);
//
void* strhash_read(u64);
void* pin_read(int);
void* chipindex_read(int);
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
//
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
	u32 desttype;	//eg: 'hash', 'dir', 'file', 'func'
	u32 destflag;
	u32 samepinprevchip;
	u32 samepinnextchip;

	u64 selfchip;
	u64 selffoot;
	u32 selftype;	//eg: 'dir', 'file', 'func', 'hash'
	u32 selfflag;
	u32 samechipprevpin;
	u32 samechipnextpin;
};
struct routestack
{
	u64 type;
	u64 data;
	void* addr;
	void* padd;
	struct wire* irel;
	struct wire* orel;
};
static struct routestack st[16];
static int rsp = 0;




void route_dfs(
	struct wire* irel, struct wire* orel,
	u64 type, u64 addr)
{
	int j;

first:
	while(1)
	{
		if(0 == irel)goto second;
		if((type == irel->selftype)&&(addr == irel->selfchip))
		{
			printf("%.4s.%llx.i\n",
				irel->desttype,
				irel->destchip
			);
			return;
		}
		irel = samepinprevchip(irel);
	}

second:
	while(1)
	{
		if(0 == orel)goto third;
		if((type == orel->desttype)&&(addr == orel->destchip))
		{
			printf("%.4s.%llx.i\n",
				orel->selftype,
				orel->selfchip
			);
			return;
		}
		orel = samechipprevpin(orel);
	}

third:
	return;
}
void route(int argc, char** argv)
{
	int j;
	u64 t1, t2;
	u64 type, data, addr;
	struct hash* h1;
	struct hash* h2;

	//check
	if(argc != 3)
	{
		printf("give me 2 names\n");
		return;
	}
	readthemall(1);

	//str1
	t1 = strhash_generate(argv[1], strlen(argv[1]));
	h1 = strhash_read(t1);
	if(0 == h1)
	{
		printf("(%016llx)%s: notfound\n", t1, argv[1]);
		return;
	}
	printf("(%016llx)%s: %llx\n", t1, argv[1], h1);

	//str2
	t2 = strhash_generate(argv[2], strlen(argv[2]));
	h2 = strhash_read(t2);
	if(0 == h2)
	{
		printf("(%016llx)%s: notfound\n", t2, argv[2]);
		return;
	}
	printf("(%016llx)%s: %llx\n", t2, argv[2], h2);

	//prepare
	st[0].type = __hash__;
	st[0].data = t1;
	st[0].addr = h1;
	st[0].irel = relation_read(h1->irel);
	st[0].orel = relation_read(h2->orel);
	rsp = 0;
#define maxdepth 8
	//doit
	while(1)
	{
		if(0 != st[rsp].irel)
		{
			type = (st[rsp].irel)->selftype;
			data = (st[rsp].irel)->selfchip;
			st[rsp].irel = samepinprevchip(st[rsp].irel);
printf("%d.i) %llx,%llx\n",rsp,type,data);

			if((__hash__ == type)&&(t2 == data))break;
			if(rsp >= maxdepth)continue;
			for(j=0;j<=rsp;j++)
			{
				if((type == st[j].type)&&(data == st[j].data))break;
			}
			if(j <= rsp)continue;

			if(__hash__ == type)h1 = strhash_read(data);
			else if(__file__ == type)h1 = filemd5_read(data);
			else if(__fun__ == type)h1 = funcindex_read(data);
			else if(__chip__ == type)h1 = chipindex_read(data);
			else if(__pin__ == type)h1 = pin_read(data);
			else continue;

			rsp++;
			st[rsp].type = type;
			st[rsp].data = data;
			st[rsp].addr = h1;
			st[rsp].irel = relation_read(h1->irel);
			st[rsp].orel = relation_read(h1->orel);
			continue;
		}

		if(0 != st[rsp].orel)
		{
			type = (st[rsp].orel)->desttype;
			data = (st[rsp].orel)->destchip;
			st[rsp].orel = samechipprevpin(st[rsp].orel);
printf("%d.o) %llx,%llx\n",rsp,type,data);

			if((__hash__ == type)&&(t2 == data))break;
			if(rsp >= maxdepth)continue;
			for(j=0;j<=rsp;j++)
			{
				if((type == st[j].type)&&(data == st[j].data))break;
			}
			if(j <= rsp)continue;

			if(__hash__ == type)h1 = strhash_read(data);
			else if(__file__ == type)h1 = filemd5_read(data);
			else if(__fun__ == type)h1 = funcindex_read(data);
			else if(__chip__ == type)h1 = chipindex_read(data);
			else if(__pin__ == type)h1 = pin_read(data);
			else continue;

			rsp++;
			st[rsp].type = type;
			st[rsp].data = data;
			st[rsp].addr = h1;
			st[rsp].irel = relation_read(h1->irel);
			st[rsp].orel = relation_read(h1->orel);
			continue;
		}

		if(0 == rsp)break;
		rsp--;
	}
	for(j=0;j<rsp;j++)
	{
		if(__hash__ == st[j].type)
		{
			strhash_print(st[j].data);
			printf(" -> ");
		}
		else
		{
			printf("%.4s,%llx -> ", &st[j].type, st[j].data);
		}
	}
	printf("%s\n", argv[2]);
}
