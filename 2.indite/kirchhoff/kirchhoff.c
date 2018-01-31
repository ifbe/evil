#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#define f32 float
#define f64 double
#define hex32(a,b,c,d) (a | (b<<8) | (c<<16) | (d<<24))
#define __hash__ hex32('h','a','s','h')
#define __chip__ hex32('c','h','i','p')
#define __pin__ hex32('p','i','n',0)
void readthemall(int);
void writethemall(int);
//
u64 strhash_generate(void*, int);
void strhash_print(u64);
void* strhash_read(u64);
void* pin_read(int);
void* chipindex_read(int);
//
void* relation_read(u64);
void* samepinprevchip(void*);
void* samepinnextchip(void*);
void* samechipprevpin(void*);
void* samechipnextpin(void*);




struct relation
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
struct hash
{
	u32 hash0;
	u32 hash1;
	u32 off;
	u32 len;

	u64 irel;
	u64 orel;
};
struct chipindex
{
	u32 self;
	u32 what;
	u32 type;
	f32 data;

	u64 irel;
	u64 orel;
};
struct pinindex
{
	u32 self;
	u32 what;
	u32 off;
	u32 len;

	u64 irel;
	u64 orel;
};




/*
static void* stack[0x100];
static int target[0x100];
static int rsp = 0;
void kirchhoff_chip(int offset)
{
	int flag;
	struct chipindex* chip;
	struct wire* irel;
	struct wire* orel;
	//if(offset%0x20 != 0)printf("notfound: chip@%x",offset);

	chip = chipindex_read(offset);
	//printf("chip@%08x	@%llx\n", offset, (u64)chip);

	orel = relation_read(chip->orel);
	if(orel == 0)return;

if(orel->selfflag == hex32('V',0,0,0))
{
	while(orel != 0)
	{
		flag = 0;
		if(orel->desttype == hex32('p','i','n',0))
		{
			flag = 1;
			printf("U%llx", (orel->destchip)/0x20);
		}

		orel = samechipnextpin(orel);
		if(orel == 0)break;

		if(flag == 1)printf(" - ");
	}
	printf(" = V%llx\n", orel->selfchip);
}//power
else if(orel->selfflag == hex32('R',0,0,0))
{
	while(orel != 0)
	{
		flag = 0;
		if(orel->desttype == hex32('p','i','n',0))
		{
			flag = 1;
			printf("U%llx", (orel->destchip)/0x20);
		}

		orel = samechipnextpin(orel);
		if(orel == 0)break;

		if(flag == 1)printf(" - ");
	}

	orel = relation_read(chip->orel);
	printf(" = R%llx * (", orel->selfchip);

	while(orel != 0)
	{
		flag = 0;
		if(orel->desttype == hex32('p','i','n',0))
		{
			flag = 1;
			printf("I%llx", orel);
			//printf("I(%llx:%c)", orel->selfchip, (u32)(orel->selffoot));
		}

		orel = samechipnextpin(orel);
		if(orel == 0)break;

		if(flag == 1)printf(" - ");
	}
	printf(")\n");
}//resistor
else if(orel->selfflag == hex32('N','M','O','S'))
{
	flag = 0;
	while(orel != 0)
	{
		flag |= 0xfc;
		if(orel->desttype == hex32('p','i','n',0))
		{
			if(orel->selffoot == 'D')
			{
				flag &= 0x3;
				flag |= 1;
				printf("U%llx", (orel->destchip)/0x20);
			}
			else if(orel->selffoot == 'S')
			{
				flag &= 0x3;
				flag |= 2;
				printf("U%llx", (orel->destchip)/0x20);
			}
		}

		orel = samechipnextpin(orel);
		if(orel == 0)break;

		if((flag == 1)|(flag == 2))printf(" = ");
	}
	printf("\n");

	orel = relation_read(chip->orel);
	flag = 0;
	while(orel != 0)
	{
		flag |= 0xfc;
		if(orel->desttype == hex32('p','i','n',0))
		{
			if(orel->selffoot == 'D')
			{
				flag &= 0x3;
				flag |= 1;
				printf("I%llx", orel);
			}
			else if(orel->selffoot == 'S')
			{
				flag &= 0x3;
				flag |= 2;
				printf("I%llx", orel);
			}
		}

		orel = samechipnextpin(orel);
		if(orel == 0)break;

		if((flag == 1)|(flag == 2))printf(" + ");
	}
	printf(" = 0\n");
}//nmos
}
void kirchhoff_pin(int offset)
{
	struct pinindex* pin;
	struct wire* irel;
	struct wire* orel;
	//if(offset%0x20 != 0)printf("notfound: pin@%x",offset);

	pin = pin_read(offset);
	//printf("pin@%08x	@%llx\n", offset, (u64)pin);

	irel = relation_read(pin->irel);
	if(irel == 0)return;

	//input
	while(irel != 0)
	{
		if(irel->selftype == hex32('c','h','i','p'))
		{
			//printf("I(%llx:%c)", irel->selfchip, (u32)(irel->selffoot));
			printf("I%llx", irel);
		}

		irel = samepinnextchip(irel);
		if(irel == 0)break;

		printf(" + ");
	}
	printf(" = 0\n");
}




void* kirchhoff_traverse_forpin(struct wire* base)
{
	struct wire* rel = base;
	while(1)
	{
		if(	(rel->desttype == hex32('p','i','n',0)) &&
			(rel->destfoot == 0) )
		{
			return rel;
		}

		rel = samechipprevpin(rel);
		if(rel == 0)break;
	}

	rel = base;
	while(1)
	{
		if(	(rel->desttype == hex32('p','i','n',0)) &&
			(rel->destfoot == 0) )
		{
			return rel;
		}

		rel = samechipnextpin(rel);
		if(rel == 0)break;
	}

	return 0;
}
void* kirchhoff_traverse_forchip(struct wire* base)
{
	struct wire* rel = base;
	while(1)
	{
		if(	(rel->selftype == hex32('c','h','i','p')) &&
			(rel->destfoot == 0) )
		{
			return rel;
		}

		rel = samepinprevchip(rel);
		if(rel == 0)break;
	}

	rel = base;
	while(1)
	{
		if(	(rel->selftype == hex32('c','h','i','p')) &&
			(rel->destfoot == 0) )
		{
			return rel;
		}

		rel = samepinnextchip(rel);
		if(rel == 0)break;
	}

	return 0;
}
void kirchhoff_traverseold(int offset)
{
	struct pinindex* pin;
	struct wire* rel;

	pin = pin_read(offset);
	if(pin == 0)return;

	rel = relation_read(pin->irel);
	if(rel == 0)return;

	rsp = 0;
	stack[rsp] = rel;
	target[rsp] = 'p';
	while(1)
	{
		//too deep, so break
		if(rsp >= 0x10)break;

		//not break, go back
		if(rel == 0)
		{
			if(rsp == 0)break;

			rsp--;
			rel = stack[rsp];
			continue;
		}

		//from pin to chip
		if(target[rsp] == 'p')
		{
			rel = kirchhoff_traverse_forpin(rel);
			if(rel == 0)continue;

			printf("%x:	%llx:%c -> pin%lld\n", rsp,
				rel->selfchip, (u32)(rel->selffoot),
				(rel->destchip)/0x20);
			rel->destfoot = 1;

			rsp++;
			target[rsp] = 'c';
			stack[rsp] = rel;

		}
		else
		{
			rel = kirchhoff_traverse_forchip(rel);
			if(rel == 0)continue;

			printf("%x:	pin%lld -> %llx:%c\n", rsp,
				(rel->destchip)/0x20,
				rel->selfchip, (u32)(rel->selffoot));
			rel->destfoot = 1;

			rsp++;
			target[rsp] = 'p';
			stack[rsp] = rel;

		}
	}
}
*/




struct context
{
	u64 type;
	union{
		u64 addr;
		f64 data;
	};
	union{
		u8 str[16];
		struct	//U, I, R
		{
			u32 P;
			u32 N;
		};
		struct	//mos
		{
			u32 D;
			u32 S;
			u32 G;
			u32 B;
		};
		struct	//pin
		{
			f32 V;
			f32 I;
		};
	};
};
struct wirectx
{
	u32 foot;
	u16 chip;
	u16 pin;
};
static struct context ctxbuf[100];
static int ctxlen;
static struct wirectx wbuf[100];
static int wlen;




int kirchhoff_add(u64 type, u64 addr)
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

	ctxlen++;
	return k;
}
void kirchhoff_wire(u32 pin, u32 chip, u32 foot)
{
	int j;
	u64 data;
	u64* addr;

	data = ((u64)pin<<48) + ((u64)chip<<32) + foot;
	addr = (void*)wbuf;
	for(j=0;j<wlen;j++)
	{
		if(addr[j] == data)return;
	}

	addr[wlen] = data;
	wlen += 1;

	//printf("	%d,%d,%c\n", pin, chip, foot);
	return;
}
void kirchhoff_bfs(int cur, int len)
{
	int j,k;
	struct hash* h;
	struct relation* w;
printf("[%d,%d)\n",cur,len);

	for(j=cur;j<len;j++)
	{
		printf("%d:%.4s,%llx\n",j,&ctxbuf[j].type, ctxbuf[j].addr);

		if(ctxbuf[j].type == __chip__)
		{
			h = chipindex_read(ctxbuf[j].addr);
			if(h == 0)continue;
		}
		else if(ctxbuf[j].type == __pin__)
		{
			h = pin_read(ctxbuf[j].addr);
			if(h == 0)continue;
		}
		else continue;

		w = relation_read(h->irel);
		while(1)
		{
			if(w == 0)break;
			if(w->selftype == __chip__)
			{
				k = kirchhoff_add(__chip__, w->selfchip);
				kirchhoff_wire(j, k, w->selffoot);
			}
			if(w->selftype == __pin__)
			{
				k = kirchhoff_add(__pin__, w->selfchip);
				kirchhoff_wire(k, j, w->destfoot);
			}
			w = samepinprevchip(w);
		}

		w = relation_read(h->orel);
		while(1)
		{
			if(w == 0)break;
			if(w->desttype == __chip__)
			{
				k = kirchhoff_add(__chip__, w->destchip);
				kirchhoff_wire(j, k, w->destfoot);
			}
			if(w->desttype == __pin__)
			{
				k = kirchhoff_add(__pin__, w->destchip);
				kirchhoff_wire(k, j, w->selffoot);
			}
			w = samechipprevpin(w);
		}
	}
}




void kirchhoff_data()
{
	int j,t;
	f32 f;
	struct chipindex* chip;
printf("\ndata:\n");

	for(j=0;j<ctxlen;j++)
	{
		if(ctxbuf[j].type == __chip__)
		{
			chip = chipindex_read(ctxbuf[j].addr);
			t = chip->type;
			f = chip->data;

			//printf("%d, %c, %f\n", j, t, f);
			ctxbuf[j].type = t;
			ctxbuf[j].data = f;
		}
	}

	for(j=0;j<ctxlen;j++)
	{
		printf("%s:	%f\n", &ctxbuf[j].type, ctxbuf[j].data);
	}
}
void kirchhoff_sort()
{
	int j,k;
	u64 temp;
	u64* addr = (void*)wbuf;
printf("\nsort:\n");

	for(j=0;j<wlen;j++)
	{
		for(k=j+1;k<wlen;k++)
		{
			if(addr[k-1] > addr[k])
			{
				temp = addr[k];
				addr[k] = addr[k-1];
				addr[k-1] = temp;
			}
		}
	}
	for(j=0;j<wlen;j++)
	{
		printf("%d,%d,%c\n",
			(addr[j]>>48)&0xffff,
			(addr[j]>>32)&0xffff,
			addr[j]&0xff);
	}
}
void kirchhoff(int argc, char** argv)
{
	int i,j,m;
	u64 temp;
	struct hash* h;
	struct relation* w;

	if(argc < 2)
	{
		printf("give me the name\n");
		return;
	}
	readthemall(1);

	temp = strhash_generate(argv[1], strlen(argv[1]));
	h = strhash_read(temp);
	if(h == 0)return;

	w = relation_read(h->irel);
	if(w == 0)return;

	wlen = 0;
	ctxlen = 0;
	kirchhoff_add(w->selftype, w->selfchip);

	j = 0;
	for(i=0;i<20;i++)
	{
		m = ctxlen;
		kirchhoff_bfs(j, ctxlen);

		if(ctxlen <= m)break;
		if(ctxlen >= 0x1000)break;

		j = m;
	}

	kirchhoff_data();
	kirchhoff_sort();
}
