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
	struct hash* sc, u64 sf, u32 sct, u32 sft,
	struct hash* dc, u64 df, u32 dct, u32 dft)
{
	struct relation* rel;
	if(wirecur >= wirelen)
	{
		lseek(wirefd, 0, SEEK_SET);
		write(wirefd, wirebuf, wirecur);

		wirelen *= 2;
		wirebuf = realloc(wirebuf, wirelen);
		printf("wirebuf realloc: %x/%x\n", wirecur, wirelen);
	}

	rel = (void*)wirebuf + wirecur;
	wirecur += sizeof(struct relation);

	//1.src
	rel->srcchiptype = sct;
	rel->srcfoottype = sft;
	rel->samesrcprevdst = 0;
	rel->samesrcnextdst = 0;

	if(_hash_ == rel->srcchiptype)rel->srcchip = *(u64*)sc;
	else rel->srcchip = *(u32*)sc;
	rel->srcfoot = sf;

	//2.dst
	rel->dstchiptype = dct;
	rel->dstfoottype = dft;
	rel->samedstprevsrc = 0;
	rel->samedstnextsrc = 0;

	if(_hash_ == rel->dstchiptype)rel->dstchip = *(u64*)dc;
	else rel->dstchip = *(u32*)dc;
	rel->dstfoot = df;

	return rel;
}




void* samedstprevsrc(struct relation* rel)
{
	u32 j;
	if(0 == rel)return 0;

	j = rel->samedstprevsrc;
	if(0 == j)return 0;
	return (void*)wirebuf + (j<<6);
}
void* samedstnextsrc(struct relation* rel)
{
	u32 j;
	if(0 == rel)return 0;

	j = rel->samedstnextsrc;
	if(0 == j)return 0;
	return (void*)wirebuf + (j<<6);
}
void* samesrcprevdst(struct relation* rel)
{
	u32 j;
	if(0 == rel)return 0;

	j = rel->samesrcprevdst;
	if(0 == j)return 0;
	return (void*)wirebuf + (j<<6);
}
void* samesrcnextdst(struct relation* rel)
{
	u32 j;
	if(0 == rel)return 0;

	j = rel->samesrcnextdst;
	if(0 == j)return 0;
	return (void*)wirebuf + (j<<6);
}
void* relationread(u32 j)
{
	if(0 == j)return 0;
	return (void*)wirebuf + (j<<6);
}
u32 relationwrite(struct relation* rel)
{
	u32 j = (void*)rel - (void*)wirebuf;
	return j>>6;
}




void* relationdelete(struct relation* rel)
{
	return 0;
}
void* relationcreate(
	void* sc, u64 sf, u32 sct, u32 sft,
	void* dc, u64 df, u32 dct, u32 dft)
{
//hashinfo, hashfoot, 'hash', fileinfo, 0, 'file'
//fileinfo, fileline, 'file', funcinfo, 0, 'func'
//funcinfo, funcofst, 'func', hashinfo, 0, 'hash'
//wininfo,  position, 'win',  actor,    0, 'act'
//actinfo,  which,    'act',  userinfo, what,     'user'
	u32 offnew;
	u32 offtmp;
	struct hash* src;
	struct hash* dst;
	struct relation* relnew;
	struct relation* reltmp;

	//new
	relnew = relation_generate(sc, sf, sct, sft, dc, df, dct, dft);
	offnew = relationwrite(relnew);

	//src
	src = sc;
	if(0 != src->oreln)
	{
		reltmp = relationread(src->oreln);
		offtmp = relationwrite(reltmp);

		reltmp->samesrcnextdst = offnew;
		relnew->samesrcprevdst = offtmp;
	}
	src->oreln = offnew;
	if(0 == src->orel0)src->orel0 = offnew;

	//dst
	dst = dc;
	if(0 != dst->ireln)
	{
		reltmp = relationread(dst->ireln);
		offtmp = relationwrite(reltmp);

		reltmp->samedstnextsrc = offnew;
		relnew->samedstprevsrc = offtmp;
	}
	dst->ireln = offnew;
	if(0 == dst->irel0)dst->irel0 = offnew;

	return relnew;
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
