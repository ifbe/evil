#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#ifndef O_BINARY
        #define O_BINARY 0x0
#endif
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#define hex32(a,b,c,d) (a | (b<<8) | (c<<16) | (d<<24))
//void* stringhash_read(u64 hash);




struct hash
{
	u32 hash0;
	u32 hash1;
	u32 off;
	u32 len;

	u64 irel;
	u64 orel;
};
struct fileindex
{
	u32 self;
	u32 what;
	u32 off;
	u32 len;

	u64 irel;
	u64 orel;
};
struct funcindex
{
	u32 self;
	u32 what;
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
	u32 destflag;		//eg: 'bad', 'ok'
	u32 samepinprevchip;
	u32 samepinnextchip;

	u64 selfchip;
	u64 selffoot;
	u32 selftype;		//eg: 'dir', 'file', 'func', 'hash'
	u32 selfflag;		//eg: 'bad', 'ok'
	u32 samechipprevpin;
	u32 samechipnextpin;
};
static u8* wirebuf;
static int wirefd;
static int wirecur;
static int wirelen;




void* connect_generate(
	struct hash* uchip, u64 ufoot, u32 utype,
	struct hash* bchip, u64 bfoot, u32 btype)
{
	struct wire* w = (void*)wirebuf + wirecur;
	wirecur += sizeof(struct wire);

	//1.dest
	if(utype == hex32('h','a','s','h'))w->destchip = *(u64*)uchip;
	else w->destchip = *(u32*)uchip;
	w->destfoot = ufoot;

	w->desttype = utype;
	w->destflag = 0;
	w->samepinprevchip = 0;
	w->samepinnextchip = 0;

	//2.self
	if(btype == hex32('h','a','s','h'))w->selfchip = *(u64*)bchip;
	else w->selfchip = *(u32*)bchip;
	w->selffoot = bfoot;

	w->selftype = btype;
	w->selfflag = 0;
	w->samechipprevpin = 0;
	w->samechipnextpin = 0;

	return w;
}




//hashinfo, hashfoot, 'hash', fileinfo, 0, 'file'
//fileinfo, fileline, 'file', funcinfo, 0, 'func'
//funcinfo, funcofst, 'func', hashinfo, 0, 'hash'
//wininfo,  position, 'win',  actor,    0, 'act'
//actinfo,  which,    'act',  userinfo, what,     'user'
int connect_write(
	void* uchip, u64 ufoot, u32 utype,
	void* bchip, u64 bfoot, u32 btype)
{
	struct hash* h1;
	struct hash* h2;
	struct wire* ww;
	struct wire* wc;
	if(wirecur >= wirelen)
	{
		lseek(wirefd, 0, SEEK_SET);
		write(wirefd, wirebuf, wirecur);

		wirelen *= 2;
		wirebuf = realloc(wirebuf, wirelen);
		printf("wirebuf realloc: %x/%x\n", wirecur, wirelen);
	}

	//
	ww = connect_generate(uchip, ufoot, utype, bchip, bfoot, btype);

	//
	h1 = uchip;
	if(h1->irel == 0)
	{
		h1->irel = (void*)ww - (void*)wirebuf;
	}
	else
	{
		wc = (void*)wirebuf + (h1->irel);
		while(wc->samepinnextchip != 0)wc = (void*)wirebuf + (wc->samepinnextchip);
		wc->samepinnextchip = (void*)ww - (void*)wirebuf;
		ww->samepinprevchip = (void*)wc - (void*)wirebuf;
	}

	h2 = bchip;
	if(h2->orel == 0)
	{
		h2->orel = (void*)ww - (void*)wirebuf;
	}
	else
	{
		wc = (void*)wirebuf + (h2->orel);
		while(wc->samechipnextpin != 0)wc = (void*)wirebuf + (wc->samechipnextpin);
		wc->samechipnextpin = (void*)ww - (void*)wirebuf;
		ww->samechipprevpin = (void*)wc - (void*)wirebuf;
	}

	return 1;

/*
	//dest wire
	h1 = uchip;
	if(h1->irel == 0)
	{
		w1 = connect_generate(uchip, ufoot, utype, 0, 0, 0);
		h1->irel = (void*)w1 - (void*)wirebuf;
		//h1->orel = 0;
	}
	else
	{
		w1 = (void*)wirebuf + (h1->irel);
		if( (w1->desttype != utype) | (w1->selftype != 0) )
		{
			wc = w1;
			w1 = connect_generate(h1, ufoot, utype, 0, 0, 0);
			h1->irel = (void*)w1 - (void*)wirebuf;

			wc->samechipprevpin = (void*)w1 - (void*)wirebuf;
			w1->samechipnextpin = (void*)wc - (void*)wirebuf;
		}
	}




	//w1->samechipprevpin = ?;		//unchanged
	//w1->samechipnextpin = ?;		//unchanged

	//w1->samepinprevchip = 0;		//certainly
	wc = w1;
	while(wc->samepinnextchip != 0)wc = (void*)wirebuf + (wc->samepinnextchip);

	wc->samepinnextchip = (void*)w2 - (void*)wirebuf;
	w2->samepinprevchip = (void*)wc - (void*)wirebuf;
	w2->samepinnextchip = 0;		//certainly





	wc = (void*)wirebuf + (h2->irel);
	while(wc->samechipnextpin != 0)wc = (void*)wirebuf + (wc->samechipnextpin);
	if(wc != w2)
	{
		wc->samechipnextpin = (void*)w2 - (void*)wirebuf;
		w2->samechipprevpin = (void*)wc - (void*)wirebuf;
		w2->samechipnextpin = 0;
	}
*/
	return 1;
}
void* connect_read(int offset)
{
	return (void*)wirebuf + offset;
}
void connect_list()
{
}
void connect_choose()
{
}
void connect_start(int type)
{
	int j;
	int flag1, flag2;
	char* name = ".42/wire/00";

	//name
	flag1 = O_CREAT|O_RDWR|O_BINARY;
	if(type == 0)flag1 |= O_TRUNC;
	flag2 = S_IRWXU|S_IRWXG|S_IRWXO;

	//open
	wirefd = open(name, flag1, flag2);
	if(wirefd <= 0)
	{
		printf("error@wirefd\n");
		exit(-1);
	}

	//malloc
	wirelen = 0x100000;
	wirebuf = malloc(wirelen);

	//read
	if(type == 0)wirecur = sizeof(struct wire);
	else
	{
		wirecur = 0;
		while(1)
		{
			j = read(wirefd, wirebuf+wirecur, wirelen-wirecur);
			if(j != wirelen-wirecur)
			{
				wirecur += j;
				break;
			}

			wirecur = wirelen;
			wirelen *= 2;
			wirebuf = realloc(wirebuf, wirelen);
		}

		printf("wirecur:	%x\n", wirecur);
		for(j=wirecur;j<wirelen;j++)wirebuf[j] = 0;
	}
}
void connect_stop()
{
	lseek(wirefd, 0, SEEK_SET);
	write(wirefd, wirebuf, wirecur);
	close(wirefd);
}
void connect_create()
{
}
void connect_delete()
{
}
