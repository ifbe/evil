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
	struct hash* uc, u64 uf, u64 ut,
	struct hash* bc, u64 bf, u64 bt)
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

	//1.dest
	rel->desttype = ut&0xffffffff;
	rel->destflag = ut>>32;
	rel->samedstprevsrc = 0;
	rel->samedstnextsrc = 0;

	if(_hash_ == rel->desttype)rel->destchip = *(u64*)uc;
	else rel->destchip = *(u32*)uc;
	rel->destfoot = uf;

	//2.self
	rel->selftype = bt&0xffffffff;
	rel->selfflag = bt>32;
	rel->samesrcprevdst = 0;
	rel->samesrcnextdst = 0;

	if(_hash_ == rel->selftype)rel->selfchip = *(u64*)bc;
	else rel->selfchip = *(u32*)bc;
	rel->selffoot = bf;

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
}
void* relationcreate(void* uc, u64 uf, u64 ut, void* bc, u64 bf, u64 bt)
{
//hashinfo, hashfoot, 'hash', fileinfo, 0, 'file'
//fileinfo, fileline, 'file', funcinfo, 0, 'func'
//funcinfo, funcofst, 'func', hashinfo, 0, 'hash'
//wininfo,  position, 'win',  actor,    0, 'act'
//actinfo,  which,    'act',  userinfo, what,     'user'
	u32 offnew;
	u32 offtmp;
	struct hash* h1;
	struct hash* h2;
	struct relation* relnew;
	struct relation* reltmp;

	//
	relnew = relation_generate(uc, uf, ut, bc, bf, bt);
	offnew = relationwrite(relnew);

	//
	h1 = uc;
	if(0 != h1->ireln)
	{
		reltmp = relationread(h1->ireln);
		offtmp = relationwrite(reltmp);

		reltmp->samedstnextsrc = offnew;
		relnew->samedstprevsrc = offtmp;
	}
	h1->ireln = offnew;
	if(0 == h1->irel0)h1->irel0 = offnew;

	h2 = bc;
	if(0 != h2->oreln)
	{
		reltmp = relationread(h2->oreln);
		offtmp = relationwrite(reltmp);

		reltmp->samesrcnextdst = offnew;
		relnew->samesrcprevdst = offtmp;
	}
	h2->oreln = offnew;
	if(0 == h2->orel0)h2->orel0 = offnew;

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
