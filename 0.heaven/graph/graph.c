#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#define hex32(a,b,c,d) (a | (b<<8) | (c<<16) | (d<<24))
#define hex64(a,b,c,d,e,f,g,h) (hex32(a,b,c,d) | (((u64)hex32(e,f,g,h))<<32))
void graph_show(
	void* vertexbuf, int vertexlen,
	u16* rectbuf, int rectlen,
	u16* tribuf, int trilen,
	u16* linebuf, int linelen,
	u16* pointbuf, int pointlen);
void readall(int);
u64 strhash_generate(void*, int);
void* strhash_read(u64);
void* shape_read(int);
void* point_read(int);
void* connect_read(int);




struct pointindex
{
	u32 self;
	float x;
	float y;
	float z;

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
struct vertex
{
	float x;
	float y;
	float z;
};
static struct vertex vertexbuf[0x10000];
static u16 rectbuf[0x10000];
static u16 tribuf[0x10000];
static u16 linebuf[0x10000];
static u16 pointbuf[0x10000];
static int vertexlen = 0;
static int rectlen = 0;
static int trilen = 0;
static int linelen = 0;
static int pointlen = 0;




void traverseshape_dfs(struct shapeindex* shape)
{
	u64 type;
	u64 temp;
	struct wire* rel;
	struct vertex* vv;
	struct shapeindex* ss;
	struct pointindex* pp;

shapeirel:
	temp = shape->irel;
	if(temp == 0)goto shapeorel;

	rel = connect_read(temp);
	if(rel == 0)goto shapeorel;

	type = shape->type;
	printf("%.8s\n", (char*)&type);
	while(rel != 0)
	{
		if(rel->selftype == hex32('s','h','a','p'))
		{
			ss = shape_read(rel->selfchip);
			traverseshape_dfs(ss);
			//printf("i:	shap@%08llx	%.8s\n",
			//rel->selfchip, &(ss->type));
		}
		else if(rel->selftype == hex32('p','o','i','n'))
		{
			temp = (rel->selfchip)/0x20;
			if(type == hex32('r','e','c','t'))
			{
				rectbuf[rectlen] = temp;
				rectlen++;
			}
			else if(type == hex32('t','r','i',0))
			{
				tribuf[trilen] = temp;
				trilen++;
			}
			else if(type == hex32('l','i','n','e'))
			{
				linebuf[linelen] = temp;
				linelen++;
			}
			else if(type == hex32('p','o','i','n'))
			{
				pointbuf[pointlen] = temp;
				pointlen++;
			}

			vv = &vertexbuf[temp];
			pp = point_read(rel->selfchip);
			vv->x = pp->x;
			vv->y = pp->y;
			vv->z = pp->z;

			printf("i:	%lld@%llx	(%f, %f, %f)\n",
				(rel->selfchip)/0x20, (u64)vv,
				vv->x, vv->y, vv->z);
		}

		temp = rel->samepinnextchip;
		if(temp == 0)break;

		rel = connect_read(temp);
		if(rel == 0)break;
	}

shapeorel:
	if(type == hex32('r','e','c','t'))
	{
		if((rectlen%4) != 0)printf("error@rectlen\n");
	}
	else if(type == hex32('t','r','i',0))
	{
		if((trilen%3) != 0)printf("error@trilen\n");
	}
	else if(type == hex32('l','i','n','e'))
	{
		if((linelen%2) != 0)printf("error@line\n");
	}
	return;
}
void* searchshapefromstr(char* buf, int len)
{
	u64 temp;
	u64 haha;
	void* shap;
	struct hash* h;
	struct wire* w;
	//printf("%.*s\n", len, buf);

	temp = strhash_generate(buf, len);
	h = strhash_read(temp);
	if(h == 0)
	{
		printf("no str: %.*s", len, buf);
		return 0;
	}

	temp = h->irel;
	w = connect_read(temp);
	if((temp == 0) | (w == 0))
	{
		printf("no rel\n");
		return 0;
	}

	haha = 0;
	while(1)
	{
		if(w->selftype == hex32('s','h','a','p'))
		{
			if(haha != 0)
			{
				printf("more than one shap\n");
				return 0;
			}
			else haha = w->selfchip;
			printf("shap@%llx\n", w->selfchip);
		}
		else
		{
			printf("%llx,%llx,%x\n", w->selfchip, w->selffoot, w->selftype);
		}

		temp = w->samepinnextchip;
		if(temp == 0)break;

		w = connect_read(temp);
		if(w == 0)break;
	}
	if(haha == 0)
	{
		printf("no shap from hash\n");
		return 0;
	}

	shap = shape_read(haha);
	return shap;
}




void graph(int argc, char** argv)
{
	char* p;
	void* temp;
	readall(1);

	if(argc == 1)p = "main";
	else p = argv[1];

	temp = searchshapefromstr(p, strlen(p));
	traverseshape_dfs(temp);

	vertexlen = 0x1000;
	graph_show(
		vertexbuf, vertexlen,
		rectbuf, rectlen,
		tribuf, trilen,
		linebuf, linelen,
		pointbuf, pointlen
	);
}