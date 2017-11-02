#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#define hex32(a,b,c,d) (a | (b<<8) | (c<<16) | (d<<24))
void* chipindex_read(int);
void* pin_read(int);
void* connect_read(int);
//
void chipdata_start(int);
void chipindex_start(int);
void pindata_start(int);
void pinindex_start(int);
void strdata_start(int);
void strhash_start(int);
void connect_start(int);




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
struct chipindex
{
	u32 self;
	u32 what;
	u32 off;
	u32 len;

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
static void* stack[0x100];
static int target[0x100];
static int rsp = 0;




void kirchhoff_chip(int offset)
{
	int flag;
	u64 temp;
	struct chipindex* chip;
	struct wire* irel;
	struct wire* orel;
	//if(offset%0x20 != 0)printf("notfound: chip@%x",offset);

	chip = chipindex_read(offset);
	//printf("chip@%08x	@%llx\n", offset, (u64)chip);

	temp = chip->orel;
	if(temp == 0)return;

	orel = connect_read(temp);
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

		temp = orel->samechipnextpin;
		if(temp == 0)break;
		//printf("temp=%x\n",temp);

		orel = connect_read(temp);
		if(orel == 0)break;
		//printf("orel=%x\n",temp);

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

		temp = orel->samechipnextpin;
		if(temp == 0)break;
		//printf("temp=%x\n",temp);

		orel = connect_read(temp);
		if(orel == 0)break;
		//printf("orel=%x\n",temp);

		if(flag == 1)printf(" - ");
	}

	temp = chip->orel;
	orel = connect_read(temp);
	printf(" = R%llx * (", orel->selfchip);

	while(orel != 0)
	{
		flag = 0;
		if(orel->desttype == hex32('p','i','n',0))
		{
			flag = 1;
			printf("I%llx", temp);
			//printf("I(%llx:%c)", orel->selfchip, (u32)(orel->selffoot));
		}

		temp = orel->samechipnextpin;
		if(temp == 0)break;
		//printf("temp=%x\n",temp);

		orel = connect_read(temp);
		if(orel == 0)break;
		//printf("orel=%x\n",temp);

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

		temp = orel->samechipnextpin;
		if(temp == 0)break;
		//printf("temp=%x\n",temp);

		orel = connect_read(temp);
		if(orel == 0)break;
		//printf("orel=%x\n",temp);

		if((flag == 1)|(flag == 2))printf(" = ");
	}
	printf("\n");

	temp = chip->orel;
	orel = connect_read(temp);
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
				printf("I%llx", temp);
			}
			else if(orel->selffoot == 'S')
			{
				flag &= 0x3;
				flag |= 2;
				printf("I%llx", temp);
			}
		}

		temp = orel->samechipnextpin;
		if(temp == 0)break;
		//printf("temp=%x\n",temp);

		orel = connect_read(temp);
		if(orel == 0)break;
		//printf("orel=%x\n",temp);

		if((flag == 1)|(flag == 2))printf(" + ");
	}
	printf(" = 0\n");
}//nmos
}
void kirchhoff_pin(int offset)
{
	u64 temp;
	struct pinindex* pin;
	struct wire* irel;
	struct wire* orel;
	//if(offset%0x20 != 0)printf("notfound: pin@%x",offset);

	pin = pin_read(offset);
	//printf("pin@%08x	@%llx\n", offset, (u64)pin);

	temp = pin->irel;
	if(temp == 0)return;

	irel = connect_read(temp);
	if(irel == 0)return;

	//input
	while(irel != 0)
	{
		if(irel->selftype == hex32('c','h','i','p'))
		{
			//printf("I(%llx:%c)", irel->selfchip, (u32)(irel->selffoot));
			printf("I%llx", temp);
		}

		temp = irel->samepinnextchip;
		if(temp == 0)break;

		irel = connect_read(temp);
		if(irel == 0)break;

		printf(" + ");
	}
	printf(" = 0\n");
}




void* kirchhoff_traverse_forpin(struct wire* base)
{
	u64 temp;
	struct wire* rel;

	rel = base;
	while(1)
	{
		if(	(rel->desttype == hex32('p','i','n',0)) &&
			(rel->destfoot == 0) )
		{
			return rel;
		}

		temp = rel->samechipprevpin;
		if(temp == 0)break;

		rel = connect_read(temp);
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

		temp = rel->samechipnextpin;
		if(temp == 0)break;

		rel = connect_read(temp);
		if(rel == 0)break;
	}

	return 0;
}
void* kirchhoff_traverse_forchip(struct wire* base)
{
	u64 temp;
	struct wire* rel;

	rel = base;
	while(1)
	{
		if(	(rel->selftype == hex32('c','h','i','p')) &&
			(rel->destfoot == 0) )
		{
			return rel;
		}

		temp = rel->samepinprevchip;
		if(temp == 0)break;

		rel = connect_read(temp);
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

		temp = rel->samepinnextchip;
		if(temp == 0)break;

		rel = connect_read(temp);
		if(rel == 0)break;
	}

	return 0;
}
void kirchhoff_traverse(int offset)
{
	u64 temp;
	struct pinindex* pin;
	struct wire* rel;

	pin = pin_read(offset);
	if(pin == 0)return;

	temp = pin->irel;
	if(temp == 0)return;

	rel = connect_read(temp);
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
void kirchhoff(int argc, char** argv)
{
	//chipdata_start(1);
	chipindex_start(1);
	//pindata_start(1);
	pinindex_start(1);
	strdata_start(1);
	strhash_start(1);
	connect_start(1);

	kirchhoff_pin(0x20);
	kirchhoff_pin(0x40);
	kirchhoff_pin(0x60);
	kirchhoff_pin(0x80);

	kirchhoff_chip(0x20);
	kirchhoff_chip(0x40);
	kirchhoff_chip(0x60);

	kirchhoff_traverse(0x20);
}