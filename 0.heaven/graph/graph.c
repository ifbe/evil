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
void readthemall(int);
void graph_init(void*, void*);
void graph_data(void*, void*);
void trianglenormal(void* n, void* a, void* b, void* c);
//
u64 strhash_generate(void*, int);
void* strhash_read(u64);
void* shapeindex_read(int);
void* pointindex_read(int);
void* pointdata_read(int);
void* connect_read(int);




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
struct vertex
{
	float x;
	float y;
	float z;
	float w;
};
static u8* origin;
static u16 tempbuf[0x1000];
static int templen = 0;
static u8 buffer[0x800000];
static u64 index[8];




void traverseshape_rectangle()
{
	u16* ii;
	float n[3];
	struct vertex* vv;

	trianglenormal(n,
		(void*)buffer + tempbuf[0]*12,
		(void*)buffer + tempbuf[1]*12,
		(void*)buffer + tempbuf[2]*12
	);
	printf("%f,%f,%f\n",n[0],n[1],n[2]);

	//normal
	vv = (void*)buffer + 0x100000 + tempbuf[0]*12;
	vv->x += n[0];
	vv->y += n[1];
	vv->z += n[2];
	vv = (void*)buffer + 0x100000 + tempbuf[1]*12;
	vv->x += n[0];
	vv->y += n[1];
	vv->z += n[2];
	vv = (void*)buffer + 0x100000 + tempbuf[2]*12;
	vv->x += n[0];
	vv->y += n[1];
	vv->z += n[2];
	vv = (void*)buffer + 0x100000 + tempbuf[3]*12;
	vv->x += n[0];
	vv->y += n[1];
	vv->z += n[2];

	//colour
	vv = (void*)buffer + 0x200000 + tempbuf[0]*12;
	vv->x = 0.5;
	vv->y = 0.28;
	vv->z = 0.0;
	vv = (void*)buffer + 0x200000 + tempbuf[1]*12;
	vv->x = 0.5;
	vv->y = 0.28;
	vv->z = 0.0;
	vv = (void*)buffer + 0x200000 + tempbuf[2]*12;
	vv->x = 0.5;
	vv->y = 0.28;
	vv->z = 0.0;
	vv = (void*)buffer + 0x200000 + tempbuf[3]*12;
	vv->x = 0.5;
	vv->y = 0.28;
	vv->z = 0.0;

	ii = (void*)buffer + 0x400000 + 2*index[4];
	ii[0] = tempbuf[0];
	ii[1] = tempbuf[1];
	ii[2] = tempbuf[2];
	ii[3] = tempbuf[3];
	index[4] += 4;
}
traverseshape_triangle()
{
	u16* ii;
	float n[3];
	struct vertex* vv;

	trianglenormal(n,
		(void*)buffer + tempbuf[0]*12,
		(void*)buffer + tempbuf[1]*12,
		(void*)buffer + tempbuf[2]*12
	);
	printf("%f,%f,%f\n",n[0],n[1],n[2]);

	//vertex
	vv = (void*)buffer + 0x100000 + tempbuf[0]*12;
	vv->x += n[0];
	vv->y += n[1];
	vv->z += n[2];
	vv = (void*)buffer + 0x100000 + tempbuf[1]*12;
	vv->x += n[0];
	vv->y += n[1];
	vv->z += n[2];
	vv = (void*)buffer + 0x100000 + tempbuf[2]*12;
	vv->x += n[0];
	vv->y += n[1];
	vv->z += n[2];

	//colour
	vv = (void*)buffer + 0x200000 + tempbuf[0]*12;
	vv->x = 0.5;
	vv->y = 0.28;
	vv->z = 0.0;
	vv = (void*)buffer + 0x200000 + tempbuf[1]*12;
	vv->x = 0.5;
	vv->y = 0.28;
	vv->z = 0.0;
	vv = (void*)buffer + 0x200000 + tempbuf[2]*12;
	vv->x = 0.5;
	vv->y = 0.28;
	vv->z = 0.0;

	ii = (void*)buffer + 0x500000 + 2*index[5];
	ii[0] = tempbuf[0];
	ii[1] = tempbuf[1];
	ii[2] = tempbuf[2];
	index[5] += 3;
}
void traverseshape_line()
{
	u16* ii;
	float n[3];
	struct vertex* vv;

	//vertex
	vv = (void*)buffer + 0x100000 + tempbuf[0]*12;
	vv->x += 0.0;
	vv->y += 0.0;
	vv->z += 1.0;
	vv = (void*)buffer + 0x100000 + tempbuf[1]*12;
	vv->x += 0.0;
	vv->y += 0.0;
	vv->z += 1.0;

	//colour
	vv = (void*)buffer + 0x200000 + tempbuf[0]*12;
	vv->x = 1.0;
	vv->y = 1.0;
	vv->z = 1.0;
	vv = (void*)buffer + 0x200000 + tempbuf[1]*12;
	vv->x = 1.0;
	vv->y = 1.0;
	vv->z = 1.0;

	ii = (void*)buffer + 0x600000 + 2*index[6];
	ii[0] = tempbuf[0];
	ii[1] = tempbuf[1];
	index[6] += 2;
}

void traverseshape_point()
{
	u16* ii;
	float n[3];
	struct vertex* vv;

	//vertex
	vv = (void*)buffer + 0x100000 + tempbuf[0]*12;
	vv->x += 0.0;
	vv->y += 0.0;
	vv->z += 1.0;

	//colour
	vv = (void*)buffer + 0x200000 + tempbuf[0]*12;
	vv->x = 1.0;
	vv->y = 1.0;
	vv->z = 1.0;

	ii = (void*)buffer + 0x700000 + 2*index[7];
	ii[0] = tempbuf[0];
	index[7] += 1;
}
void traverseshape_dfs(struct shapeindex* shape)
{
	u64 type;
	u64 temp;
	struct wire* rel;
	struct shapeindex* ss;
	struct pointindex* pp;
	struct vertex* uu;
	struct vertex* vv;

shapeirel:
	temp = shape->irel;
	if(temp == 0)return;

	rel = connect_read(temp);
	if(rel == 0)return;

	type = shape->type;
	printf("%.8s\n", (char*)&type);

	templen = 0;
	while(rel != 0)
	{
		if(rel->selftype == hex32('s','h','a','p'))
		{
			ss = shapeindex_read(rel->selfchip);
			traverseshape_dfs(ss);
			//printf("i:	shap@%08llx	%.8s\n",
			//rel->selfchip, &(ss->type));
		}
		else if(rel->selftype == hex32('p','o','i','n'))
		{
			temp = (rel->selfchip)/0x20;
			tempbuf[templen] = temp;
			templen++;

			pp = pointindex_read(rel->selfchip);
			uu = (void*)origin + pp->ofst;
			vv = (void*)buffer + temp*12;
			vv->x = uu->x;
			vv->y = uu->y;
			vv->z = uu->z;

			printf("i:	%lld@%llx	(%f, %f, %f)\n",
				(rel->selfchip)/0x20, (u64)vv, vv->x, vv->y, vv->z);
		}

		temp = rel->samepinnextchip;
		if(temp == 0)break;

		rel = connect_read(temp);
		if(rel == 0)break;
	}

shapeorel:
	if(type == hex32('r','e','c','t'))traverseshape_rectangle();
	else if(type == hex32('t','r','i',0))traverseshape_triangle();
	else if(type == hex32('l','i','n','e'))traverseshape_line();
	else if(type == hex32('p','o','i','n'))traverseshape_point();
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

	shap = shapeindex_read(haha);
	return shap;
}




void graph_one(char* buf, int len)
{
	int j;
	void* temp;
	for(j=0;j<8;j++)index[j] = 0;
	for(j=0;j<0x800000;j++)buffer[j] = 0;

	temp = searchshapefromstr(buf, len);
	traverseshape_dfs(temp);

	index[0] = 0x1000;
	index[1] = 0x1000;
	index[2] = 0x1000;
	graph_data(buffer, index);
}
void graph(int argc, char** argv)
{
	char buf[0x1000];
	char* p;
	int j;

	readthemall(1);
	origin = pointdata_read(0);

	graph_init(buffer, index);
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