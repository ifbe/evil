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
#define _wire_ hex32('l','i','n','e')
#define _tri_ hex32('t','r','i',0)
#define _rect_ hex32('r','e','c','t')
int hexstr2data(void*, void*);
//
void render_init();
void render_free();
void render_data(void*, int, void*, int);
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




struct context
{
	u64 type;
	u64 addr;
	u8 str[16];
};
static struct context nodebuf[0x100000/0x20];
static int nodelen = 0;
static u32 wirebuf[0x100000/4];
static int wirelen = 0;




int render_addnode(u64 type, u64 addr)
{
	int j,k;

	k = nodelen;
	for(j=0;j<k;j++)
	{
		if(nodebuf[j].type != type)continue;
		if(nodebuf[j].addr != addr)continue;
		return j;
	}

	nodebuf[k].type = type;
	nodebuf[k].addr = addr;
	bzero(nodebuf[k].str, 16);
	if(type == _hash_)strhash_export(addr, nodebuf[k].str, 16);

	nodelen++;
	return k;
}
int render_addpair(int j, int k)
{
	int i;
	u32 data;

	if(j==k)return 0;	//self to self
	//j = 2*j;
	//k = 2*k+1;
	if(j < k)data = (j<<16)+k;
	else data = (k<<16)+j;

	for(i=0;i<wirelen;i++){
		if(wirebuf[i] == data)return 0;
	}
	printf("wire_%x:%x,%x\n", wirelen, j, k);

	wirebuf[wirelen] = data;
	wirelen += 1;
	return 1;
}
void render_bfs(int cur, int end)
{
	int j,k;
	struct hash* h;
	struct relation* w;
printf("[%d,%d)\n",cur,end);

	for(j=cur;j<end;j++)
	{
printf("node_%x:%llx,%llx\n",j,nodebuf[j].type, nodebuf[j].addr);
		if(_hash_ == nodebuf[j].type)
		{
			h = strhash_read(nodebuf[j].addr);
			if(h == 0)continue;
		}
		else if(_file_ == nodebuf[j].type)
		{
			h = filemd5_read(nodebuf[j].addr);
			if(h == 0)continue;
		}
		else if(_func_ == nodebuf[j].type)
		{
			h = funcindex_read(nodebuf[j].addr);
			if(h == 0)continue;
		}
		else if(_chip_ == nodebuf[j].type)
		{
			h = chip_read(nodebuf[j].addr);
			if(h == 0)continue;
		}
		else if(_pin_ == nodebuf[j].type)
		{
			h = pin_read(nodebuf[j].addr);
			if(h == 0)continue;
		}
		else continue;

		w = relationread(h->irel0);
		while(1)
		{
			if(w == 0)break;
			k = render_addnode(w->srcchiptype, w->srcchip);
			if(j != k)render_addpair(j,k);

			w = samedstnextsrc(w);
		}

		w = relationread(h->orel0);
		while(1)
		{
			if(w == 0)break;
			k = render_addnode(w->dstchiptype, w->dstchip);
			if(j != k)render_addpair(k, j);

			w = samesrcnextdst(w);
		}
	}

}
void render_trav(char* buf, int len, int* nlen, int* wlen)
{
	int i,cur;
	int m,n;
	u64 type;
	u64 addr;
	printf("@render_trav:%.*s\n", len,buf);

	nodelen = 0;
	wirelen = 0;

	if(strncmp((void*)buf, "file@", 5)==0){
		hexstr2data(buf + 5, &addr);
		type = _file_;
	}
	else if(strncmp((void*)buf, "func@", 5)==0){
		hexstr2data(buf + 5, &addr);
		type = _func_;
	}
	else{
		addr = strhash_generate(buf, len);
		type = _hash_;
	}
	printf("type=%llx,addr=%llx\n",type,addr);
	render_addnode(type, addr);

	cur = 0;
	for(i=0;i<2;i++)	//2 layer
	{
		m = nodelen;
		n = wirelen;

//printf("before=%d\n", wirelen);
		render_bfs(cur, nodelen);
//printf("after=%d\n", wirelen);

		if(wirelen >= 0x1000)
		{
			nodelen = m;
			wirelen = n;
			break;
		}
		if(nodelen <= m)break;
		if(nodelen >= 1000)break;

		cur = m;
	}

	*nlen = nodelen;
	*wlen = wirelen;
}
void render(int argc, char** argv)
{
	char buf[0x1000];
	char* p;
	int j;
	int nlen,wlen;

	readthemall(1);
	render_init();

	for(j=1;j<argc;j++)
	{
		render_trav(argv[j], strlen(argv[j]), &nlen, &wlen);
		render_data(nodebuf, nodelen, wirebuf, wirelen);
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
		if(j != 0){
			render_trav(buf, j, &nlen, &wlen);
			render_data(nodebuf, nodelen, wirebuf, wirelen);
		}
	}

	render_free();
}
