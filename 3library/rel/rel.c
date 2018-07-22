#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "evil.h"
#ifndef O_BINARY
        #define O_BINARY 0x0
#endif




static u8* wirebuf;
static int wirefd;
static int wirecur;
static int wirelen;




void* relation_generate(
	struct hash* uchip, u64 ufoot, u64 utype,
	struct hash* bchip, u64 bfoot, u64 btype)
{
	struct relation* w = (void*)wirebuf + wirecur;
	wirecur += sizeof(struct relation);

	//1.dest
	w->desttype = utype&0xffffffff;
	w->destflag = utype>>32;
	w->samedstprevsrc = 0;
	w->samedstnextsrc = 0;

	if(w->desttype == hex32('h','a','s','h'))w->destchip = *(u64*)uchip;
	else w->destchip = *(u32*)uchip;
	w->destfoot = ufoot;

	//2.self
	w->selftype = btype&0xffffffff;
	w->selfflag = btype>>32;
	w->samesrcprevdst = 0;
	w->samesrcnextdst = 0;

	if(w->selftype == hex32('h','a','s','h'))w->selfchip = *(u64*)bchip;
	else w->selfchip = *(u32*)bchip;
	w->selffoot = bfoot;

	return w;
}




//hashinfo, hashfoot, 'hash', fileinfo, 0, 'file'
//fileinfo, fileline, 'file', funcinfo, 0, 'func'
//funcinfo, funcofst, 'func', hashinfo, 0, 'hash'
//wininfo,  position, 'win',  actor,    0, 'act'
//actinfo,  which,    'act',  userinfo, what,     'user'
int relation_write(
	void* uchip, u64 ufoot, u64 utype,
	void* bchip, u64 bfoot, u64 btype)
{
	struct hash* h1;
	struct hash* h2;
	struct relation* ww;
	struct relation* wc;
	if(wirecur >= wirelen)
	{
		lseek(wirefd, 0, SEEK_SET);
		write(wirefd, wirebuf, wirecur);

		wirelen *= 2;
		wirebuf = realloc(wirebuf, wirelen);
		printf("wirebuf realloc: %x/%x\n", wirecur, wirelen);
	}

	//
	ww = relation_generate(uchip, ufoot, utype, bchip, bfoot, btype);

	//
	h1 = uchip;
	if(0 != h1->ireln)
	{
		wc = (void*)wirebuf + (h1->ireln);
		wc->samedstnextsrc = (void*)ww - (void*)wirebuf;
		ww->samedstprevsrc = (void*)wc - (void*)wirebuf;
	}
	h1->ireln = (void*)ww - (void*)wirebuf;
	if(0 == h1->irel0)h1->irel0 = h1->ireln;

	h2 = bchip;
	if(0 != h2->oreln)
	{
		wc = (void*)wirebuf + (h2->oreln);
		wc->samesrcnextdst = (void*)ww - (void*)wirebuf;
		ww->samesrcprevdst = (void*)wc - (void*)wirebuf;
	}
	h2->oreln = (void*)ww - (void*)wirebuf;
	if(0 == h2->orel0)h2->orel0 = h2->oreln;

	return 1;
}




void* samedstprevsrc(struct relation* w)
{
	if(w == 0)return 0;
	if(w->samedstprevsrc == 0)return 0;
	return (void*)wirebuf + (w->samedstprevsrc);
}
void* samedstnextsrc(struct relation* w)
{
	if(w == 0)return 0;
	if(w->samedstnextsrc == 0)return 0;
	return (void*)wirebuf + (w->samedstnextsrc);
}
void* samesrcprevdst(struct relation* w)
{
	if(w == 0)return 0;
	if(w->samesrcprevdst == 0)return 0;
	return (void*)wirebuf + (w->samesrcprevdst);
}
void* samesrcnextdst(struct relation* w)
{
	if(w == 0)return 0;
	if(w->samesrcnextdst == 0)return 0;
	return (void*)wirebuf + (w->samesrcnextdst);
}
void* relation_read(int offset)
{
	if(offset == 0)return 0;
	return (void*)wirebuf + offset;
}




void relation_list()
{
}
void relation_choose()
{
}
void relation_start(int type)
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
	if(type == 0)wirecur = sizeof(struct relation);
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
void relation_stop()
{
	lseek(wirefd, 0, SEEK_SET);
	write(wirefd, wirebuf, wirecur);
	close(wirefd);
}
void relation_create()
{
}
void relation_delete()
{
}
