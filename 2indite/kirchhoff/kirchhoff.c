#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "evil.h"
void readthemall(int);
void writethemall(int);
//
u64 strhash_generate(void*, int);
void strhash_print(u64);
void* strhash_read(u64);
void* pin_read(int);
void* chip_read(int);




struct detail
{
	u64 type;
	union{
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
struct context
{
	u64 type;
	u64 addr;
	union{
		u64 name;
		u8 str[16];
	};
};
struct wirectx
{
	u32 foot;
	u16 chip;
	u16 pin;
};
static struct context ctxbuf[100];
static struct detail detail[100];
static struct wirectx wbuf[100];
static int ctxlen;
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
		printf("%d:%.4s,%llx\n", j,
			(void*)&ctxbuf[j].type, ctxbuf[j].addr);

		if(ctxbuf[j].type == _chip_)
		{
			h = chip_read(ctxbuf[j].addr);
			if(h == 0)continue;
		}
		else if(ctxbuf[j].type == _pin_)
		{
			h = pin_read(ctxbuf[j].addr);
			if(h == 0)continue;
		}
		else continue;

		w = relationread(h->irel0);
		while(1)
		{
			if(w == 0)break;
			if(w->selftype == _chip_)
			{
				k = kirchhoff_add(_chip_, w->selfchip);
				kirchhoff_wire(j, k, w->selffoot);
			}
			if(w->selftype == _pin_)
			{
				k = kirchhoff_add(_pin_, w->selfchip);
				kirchhoff_wire(k, j, w->destfoot);
			}
			w = samedstnextsrc(w);
		}

		w = relationread(h->orel0);
		while(1)
		{
			if(w == 0)break;
			if(w->desttype == _chip_)
			{
				k = kirchhoff_add(_chip_, w->destchip);
				kirchhoff_wire(j, k, w->destfoot);
			}
			if(w->desttype == _pin_)
			{
				k = kirchhoff_add(_pin_, w->destchip);
				kirchhoff_wire(k, j, w->selffoot);
			}
			w = samesrcnextdst(w);
		}
	}
}




u64 kirchhoff_name(void* addr)
{
	struct relation* w;
	struct pinindex* pin;
	if(addr == 0)return 0;

	pin = addr;
	w = relationread(pin->orel0);
	while(1)
	{
		if(w == 0)break;
		if(w->desttype == _hash_)return w->destchip;

		w = samesrcnextdst(w);
	}
	return 0;
}
void kirchhoff_data()
{
	int j;
	f32 f;
	struct chipindex* chip;
	struct pinindex* pin;
printf("\nname:\n");

	for(j=0;j<ctxlen;j++)
	{
		if(ctxbuf[j].type == _chip_)
		{
			chip = chip_read(ctxbuf[j].addr);
			ctxbuf[j].name = kirchhoff_name(chip);

			detail[j].type = chip->type;
			detail[j].data = chip->data;

			printf("%2d)%8.8s:	%.8s=%f\n",
				j, (void*)&ctxbuf[j].name,
				(void*)&detail[j].type, detail[j].data
			);
		}
		else if(ctxbuf[j].type == _pin_)
		{
			pin = pin_read(ctxbuf[j].addr);
			ctxbuf[j].name = kirchhoff_name(pin);

			detail[j].type = '?';
			detail[j].V = 0.0;

			printf("%2d)%8.8s:	%.8s\n",
				j, (void*)&ctxbuf[j].name,
				(void*)&detail[j].type
			);
		}
	}
}
void kirchhoff_sort()
{
	int j, k, pin, chip;
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

	k = -1;
	for(j=0;j<wlen;j++)
	{
		pin = wbuf[j].pin;
		chip = wbuf[j].chip;
		temp = wbuf[j].foot;

		if(pin != k)detail[pin].P = j;
		k = pin;

		if('+' == temp)detail[chip].P = pin;
		else if('-' == temp)detail[chip].N = pin;

		printf("%2d)%8.8s,%.8s,%c\n",
			j,
			(void*)&ctxbuf[pin].name,
			(void*)&ctxbuf[chip].name,
			(u32)temp
		);
	}
}
void kirchhoff_info()
{
	int j;
	printf("\nconn:\n");

	for(j=0;j<ctxlen;j++)
	{
		if(_pin_ == ctxbuf[j].type)
		{
			printf("%2d)%8.8s:	%d\n",
				j, (void*)&ctxbuf[j].name,
				detail[j].P
			);
		}
		else
		{
			printf("%2d)%8.8s:	P@%d,N@%d	%.8s=%f\n",
				j, (void*)&ctxbuf[j].name,
				detail[j].P, detail[j].N,
				(void*)&detail[j].type, detail[j].data
			);
		}
	}
}
float kirchhoff_incurr(int chip, int foot)
{
	int pin;
	if('I' == detail[chip].type)
	{
		if('-' == foot)return -detail[chip].data;
		else if('+' == foot)return detail[chip].data;
	}
	else if('R' == detail[chip].type)
	{
		if('-' == foot)pin = detail[chip].P;
		if('+' == foot)pin = detail[chip].N;
		return detail[pin].V / detail[chip].data;
	}
	else if('V' == detail[chip].type)
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
					(void*)&ctxbuf[pin].name,
					sc, di, di/sc
				);
			}
			pin = wbuf[j].pin;
			sc = 0.0;
			di = 0.0;
			if(j==wlen)break;
		}

		chip = wbuf[j].chip;
		di += kirchhoff_incurr(chip, wbuf[j].foot);
		if(detail[chip].type == 'R')
		{
			//printf("R=%f\n",detail[chip].data);
			sc += 1.0 / detail[chip].data;
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

	w = relationread(h->irel0);
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
	kirchhoff_info();
return;
	for(j=0;j<10;j++)
	{
		printf("\n%d\n", j);
		kirchhoff_iter();
	}
}
