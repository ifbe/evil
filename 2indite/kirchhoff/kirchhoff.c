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
struct context
{
	u64 type;
	union{
		u64 addr;
		u64 name;
		struct	//U, I, R
		{
			u16 P;
			u16 N;
		};
		struct	//bjt
		{
			u16 E;
			u16 C;
			u16 B;
		};
		struct	//mos
		{
			u16 D;
			u16 S;
			u16 G;
			u16 X;
		};
	};
	union{
		u8 str[16];
		struct{	//chip
			f32 data;
		};
		struct	//pin
		{
			f32 V;
			f32 I;
			f32 Vbias;
			f32 Ibias;
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




u64 kirchhoff_name(struct pinindex* pin)
{
	struct relation* w;
	if(pin == 0)return 0;

	w = relation_read(pin->orel);
	while(1)
	{
		if(w == 0)break;
		if(w->desttype == __hash__)return w->destchip;

		w = samechipprevpin(w);
	}
	return 0;
}
void kirchhoff_info()
{
	int j,t;
	f32 f;
	struct chipindex* chip;
	struct pinindex* pin;
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

			printf("%d,%s:	%f\n",
				j, &ctxbuf[j].type, ctxbuf[j].data);
		}
		else if(ctxbuf[j].type == __pin__)
		{
			pin = pin_read(ctxbuf[j].addr);
			ctxbuf[j].name = kirchhoff_name(pin);

			printf("%d,%s:	%llx\n",
				j, &ctxbuf[j].type, ctxbuf[j].name);
		}
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
float kirchhoff_incurr(int chip, int foot)
{
	int pin;
	if('I' == ctxbuf[chip].type)
	{
		if('-' == foot)return -ctxbuf[chip].data;
		else if('+' == foot)return ctxbuf[chip].data;
	}
	else if('R' == ctxbuf[chip].type)
	{
		if('-' == foot)pin = ctxbuf[chip].P;
		if('+' == foot)pin = ctxbuf[chip].N;
		return ctxbuf[pin].V / ctxbuf[chip].data;
	}
	else if('V' == ctxbuf[chip].type)
	{
		
	}
	return 0.0;
}
void kirchhoff_iter()
{
	int j;
	int pin;
	int chip;
	float sc, di;

	pin = -1;
	for(j=0;j<=wlen;j++)
	{
		if((j==wlen)|(pin != wbuf[j].pin))
		{
			if(pin >= 0)
			{
				printf(
					"%.8s:	C=%f,I=%f,V=%f\n",
					&ctxbuf[pin].name, sc, di, di/sc
				);
			}
			pin = wbuf[j].pin;
			sc = 0.0;
			di = 0.0;
			if(j==wlen)break;
		}

		chip = wbuf[j].chip;
		di += kirchhoff_incurr(chip, wbuf[j].foot);
		if(ctxbuf[chip].type == 'R')
		{
			//printf("R=%f\n",ctxbuf[chip].data);
			sc += 1.0 / ctxbuf[chip].data;
		}
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

	kirchhoff_info();
	kirchhoff_sort();

	for(j=0;j<10;j++)
	{
		printf("\n%d\n", j);
		kirchhoff_iter();
	}
}
